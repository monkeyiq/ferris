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

    $Id: EAQuery.hh,v 1.8 2011/05/03 21:30:21 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EAQUERY_H_
#define _ALREADY_INCLUDED_FERRIS_EAQUERY_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/EAIndexer.hh>
#include <Ferris/FullTextIndexer.hh>

#include <string>

namespace Ferris
{
    namespace EAIndex 
    {
        using namespace ::Ferris::FullTextIndex;

        FERRISEXP_API bool shouldRunEAQueryFilesystemAsync();
        FERRISEXP_API void shouldRunEAQueryFilesystemAsync( bool v );

        
        /**
         * Execute the given query against either the given
         * index or the default index if no index is given
         */
        FERRISEXP_API fh_context ExecuteQuery( const std::string& qs, fh_idx idx = 0 );
        FERRISEXP_API fh_context ExecuteQuery( const std::string& qs, int limit );
        /**
         * execute a query against the default ea index with the given limit
         * placing the total number of possible matches into totalNumberOfMatches
         * WARNING: This method has to execute the query to docids using no limit
         * to find out how many total matches are possible.
         */
        FERRISEXP_API fh_context ExecuteQuery( const std::string& qs, int limit, long& totalNumberOfMatches );

        /**
         * execute a query against the default ea index with the given limit
         * placing true into morePossible if there are more matches in the index
         * than limit allowed.
         */
        FERRISEXP_API fh_context ExecuteQuery( const std::string& qs, int limit, bool& morePossible );
        
        /**
         * See ExecuteQuery( const std::string& qs, fh_idx idx = 0 ) for the function you want.
         * This function allows you to pass in the selection context which will be the parent
         * for all the context's matching the query.
         *
         * Using this function allows you to set custom policy on how to handle two
         * subcontexts which have the same RDN but different URLs
         */
        FERRISEXP_API fh_context ExecuteQuery( const std::string& qs, fh_idx idx,
                                               fh_context selectionCtx,
                                               int limit = 0 );

        FERRISEXP_API docNumSet_t& ExecuteQueryToDocIDs( const std::string& qs,
                                                         fh_idx idx,
                                                         docNumSet_t& ret,
                                                         int limit = 0 );
        FERRISEXP_API stringset_t& ExecuteQueryToToURLs( const std::string& qs,
                                                         fh_idx idx,
                                                         stringset_t& ret,
                                                         int limit = 0 );

        /**
         * These are like ExecuteQuery but will spawn a subprocess to execute the query
         * and will read the results from the subprocess in a GLib2 friendly way.
         */
        FERRISEXP_API fh_context ExecuteQueryAsync( const std::string& qs, fh_idx idx = 0 );
        FERRISEXP_API fh_context ExecuteQueryAsync( const std::string& qs, int limit, fh_idx idx );
        FERRISEXP_API fh_context ExecuteQueryAsync( const std::string& qs, fh_idx idx,
                                                    fh_context selectionCtx,
                                                    int limit = 0 );
        

        namespace Factory 
        {
            /**
             * Used for more advanced queries than ExecuteQuery() can provide.
             * first call here to create a Query object and then you can setup
             * the details of the query and other tweaks that are set to default
             * values if ExecuteQuery() is used.
             */
            FERRISEXP_API fh_eaquery makeEAQuery( const std::string& qs, fh_idx idx = 0 );
        };


        class FERRISEXP_API EAQuery
            :
            public Handlable
        {
            typedef EAQuery    _Self;
            typedef Handlable  _Base;

        protected:
            
            fh_idx m_idx;
            int    m_resultSetMaxSize;
            std::string m_queryString;

            EAQuery( const std::string& s, fh_idx idx );

            friend class EAIndexManagerDB4;
            virtual docNumSet_t&
            ExecuteQuery( fh_context q, docNumSet_t& docnums, fh_db4idx idx, int limit = 0 )
                {
                    return docnums;
                }

            virtual std::string resolveDocumentIDAttemptFromCache( docid_t docid, std::map< docid_t, std::string >& cache );
            
            
        public:

            virtual ~EAQuery();

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
             * Get the matching document numbers for the query
             * @param ret set to insert docids into and use as return value
             * @param qs query string
             * @param idx index to use or the default for this user if null
             */
            virtual docNumSet_t& ExecuteQueryToDocIDs( docNumSet_t& ret ) = 0;

            /**
             * Same as ExecuteQueryToDocIDs( docNumSet_t& ret ) but allows
             * a set maximum number of files to be returned.
             */
            virtual docNumSet_t& ExecuteQueryToDocIDs( docNumSet_t& ret, int limit ) = 0;
            
            /**
             * Resolve a set of documentIDs to a context containing all
             * the fh_context's for those documents
             */
            virtual fh_context ResolveDocIDs( docNumSet_t& s ) = 0;

            /**
             * Convert a list of docids to the URL for each docid from the
             * index. This is similar to ResolveDocIDs() but some URLs might
             * not actually exist in the current filesystem and this call
             * will not filter out those non existent URLs.
             */
            virtual stringset_t& ResolveDocIDsToURLs( docNumSet_t& s, stringset_t& ret ) = 0;
            
            /**
             * Allow the caller to pass in the selection context.
             * This is a specialization of ResolveDocIDs( docNumSet_t& s )
             * to let the caller setup properties on the selection context
             * (ie, the return value). This method was created to allow views
             * to determine how to resolve the case where two files have the
             * same name but different URLs.
             */
            virtual fh_context ResolveDocIDs( docNumSet_t& s, fh_context selectionCtx ) = 0;
            
            /**
             * run the query. The result is semantically equal to
             * ResolveDocIDs( ExecuteQueryToDocIDs() )
             */
            virtual fh_context execute() = 0;

            /**
             * Allow the caller to pass in the selection context.
             * See ResolveDocIDs( docNumSet_t& s, fh_context selectionCtx ).
             */
            virtual fh_context execute( fh_context selectionCtx, int limit = 0 ) = 0;


            /**
             * the same as execute(), but spawns a subprocess and works out the query
             * results in a glib2 friendly way from the subproc executing the query
             * itself.
             */
            virtual fh_context executeAsync( fh_context selc, int limit ) = 0;
            
            
            /**
             * This calls BuildQuerySQL() to get at the raw SQL strings
             * that an ffilter will use when run against an SQL database
             * backend plugin. This method is mainly useful for FCA code.
             */
            virtual void
            getQuerySQL( std::stringstream& SQLHeaderSS,
                         std::stringstream& SQLWherePredicatesSS,
                         std::stringstream& SQLTailerSS,
                         stringset_t& lookupTablesUsed,
                         bool& queryHasTimeRestriction,
                         std::string& DocIDColumn,
                         stringset_t& eanamesUsed,
                         MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo );
            
            /**
             * only take the top v documents from the ranked results
             */
            void setResultSetMaxSize( int v );
        };
        
    };
};
#endif
