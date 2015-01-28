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

    $Id: FullTextQuery.hh,v 1.7 2010/09/24 21:30:52 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FULLTEXTQUERY_H_
#define _ALREADY_INCLUDED_FERRIS_FULLTEXTQUERY_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/FullTextIndexer.hh>

#include <string>

namespace Ferris
{
    /**
     * Support for full text indexing and querys
     */
    namespace FullTextIndex 
    {
        FERRISEXP_API bool shouldRunFullTextQueryFilesystemAsync();
        FERRISEXP_API void shouldRunFullTextQueryFilesystemAsync( bool v );
        
        FERRISEXP_API enum QueryMode {
            QUERYMODE_USERPREF = 1<<1,
            QUERYMODE_BOOLEAN  = 1<<2,
            QUERYMODE_RANKED   = 1<<3,
            QUERYMODE_XAPIAN   = 1<<4,
            QUERYMODE_TSEARCH2 = 1<<5,
            QUERYMODE_EXTERNAL = 1<<6,
            QUERYMODE_BEAGLE   = 1<<7
        };
        FERRISEXP_API std::string makeFullTextQueryFilterString( const std::string& qs,
                                                                 QueryMode qm,
                                                                 fh_idx idx );
        
        /**
         * Execute the given query in the mode selected against either the given
         * index or the default index if no index is given
         */
        FERRISEXP_API fh_context ExecuteQuery( const std::string& s,
                                               QueryMode qm = QUERYMODE_USERPREF,
                                               fh_idx idx = 0,
                                               int limit = 0 );
        FERRISEXP_API fh_context ExecuteQueryAsync( const std::string& s,
                                                    QueryMode qm = QUERYMODE_USERPREF,
                                                    fh_idx idx = 0,
                                                    int limit = 0 );

        FERRISEXP_API fh_context MountBooleanQueryString( const std::string& v );
        FERRISEXP_API docNumSet_t& ExecuteBooleanQuery( fh_context q,
                                                        docNumSet_t& docnums,
                                                        fh_idx idx,
                                                        int limit );
        

        
        class FullTextQuery;
        FERRIS_SMARTPTR( FullTextQuery, fh_ftquery );

        namespace Factory 
        {
            /**
             * Used for more advanced queries than ExecuteQuery() can provide.
             * first call here to create a Query object and then you can setup
             * ranked query cutoffs and other tweaks that are set to default
             * values if ExecuteQuery() is used.
             */
            FERRISEXP_API fh_ftquery makeFullTextQuery( const std::string& s,
                                                        QueryMode qm = QUERYMODE_USERPREF,
                                                        fh_idx idx = 0,
                                                        int limit = 0 );
        };
        
        class FERRISEXP_API FullTextQuery
            :
            public Handlable
        {
            typedef FullTextQuery _Self;
            typedef Handlable     _Base;

        protected:
            
            fh_idx m_idx;
            int    m_accumulatorsMaxSize;
            int    m_resultSetMaxSize;
            std::string m_queryString;
            fh_context m_asyncSelection;
            fh_FerrisSlaveProcess m_asyncSlave;
            int    m_limit;

            FullTextQuery( const std::string& s, fh_idx idx );

            void OnAsyncXMLMessage( fh_xstreamcol h );
            void OnAsyncChildComeplete( ChildStreamServer* css, fh_runner r, int status, int estatus );
            
        public:

            virtual ~FullTextQuery();

            /********************************************************************************/
            /********************************************************************************/
            /*** the following have effect for all query types ******************************/
            /********************************************************************************/
            /********************************************************************************/

            /**
             * set the index to use for future executions
             */
            void setIndex( fh_idx idx );
            
            /**
             * set the query string
             */
            void setQuery( const std::string& s );

            /**
             * Limit number of results, or 0 for no limit.
             */
            void setLimit( int limit = 0 );
            
            /**
             * run the query
             */
            virtual fh_context execute() = 0;

            /**
             * run the query as a subprocess
             */
            virtual fh_context executeAsync();
            
            /********************************************************************************/
            /********************************************************************************/
            /*** the following are only effective for RANKED queries ************************/
            /********************************************************************************/
            /********************************************************************************/

            /**
             * max Number of candidate documents to calculate for
             */
            void setAccumulatorsMaxSize( int v );

            /**
             * only take the top v documents from the ranked results
             */
            void setResultSetMaxSize( int v );
        };
    };
};
#endif
