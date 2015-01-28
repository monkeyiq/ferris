/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: Ferris_private.hh,v 1.10 2010/09/24 21:30:49 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_PRIV_H_

#include <EAN.hh>
#include <Ferris.hh>
#include <Enamel.hh>
#include <Ferrisls_AggregateData.hh>
#include <ValueRestorer.hh>    
#include <ChainedViewContext.hh>
#include <FilteredContext.hh>
#include <Context.hh>
#include <Configuration_private.hh>
#include <Context_private.hh>

#include <map>

namespace Ferris
{

    extern const std::string FERRIS_CONFIG_APPS_DIR;
    extern const std::string FERRIS_CONFIG_EVENT_DIR;
    extern const std::string FERRIS_CONFIG_MIMEBIND_DIR;
    extern const std::string EANAME_SL_EMBLEM_PREKEY; // "emblem:has-";
    extern const std::string EANAME_SL_EMBLEM_ID_PREKEY; // "emblem:has-";
    extern const std::string EANAME_SL_EMBLEM_TIME_PREKEY; // = "emblem:";
    extern const std::string EANAME_SL_EMBLEM_FUZZY_PREKEY;// = "emblem:has-fuzzy-";

    FERRISEXP_API std::string
    adjustRecommendedEAForDotFiles( Context* c, const std::string& s );
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    struct FERRISEXP_DLLLOCAL items_lookup_compare
    {
        inline bool operator()( fh_context s1, fh_context s2 ) const
            {
                return s1->getDirName() < s2->getDirName();
            }
        inline bool operator()( fh_context s1, const std::string s2 ) const
            {
                return s1->getDirName() < s2;
            }
        inline bool operator()( const std::string s1, fh_context s2 ) const
            {
                return s1 < s2->getDirName();
            }
    };

    FERRISEXP_DLLLOCAL void SL_FlushAggregateData( Context* c,
                                                   const std::string& rdn, EA_Atom* atom,
                                                   fh_istream ss );
    FERRISEXP_API fh_display_aggdata getCachedContextAggregateData( fh_context c, int m );
    
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    
    /**
     * Dumps debug data about all the known contexts and reference counts to ss
     */
    FERRISEXP_API void dumpEntireContextListMemoryManagementData( fh_ostream ss );

    typedef std::map< Context*, int > debug_mm_contexts_t;
    FERRISEXP_API debug_mm_contexts_t& getMMCtx();
    FERRISEXP_API void addContextToMemoryManagementData( Context* c );
    FERRISEXP_API void remContextToMemoryManagementData( Context* c );


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/


    FERRISEXP_API fh_context SL_SubCreate_file( fh_context c, fh_context md );
    

    namespace Main
    {
        /**
         * Check FDs that are registered with ferris for VFS modules and
         * process data on them. This is used for example by ContextIterator
         * to clear any queues of 'changes' to a VFS module before a iteration
         * operation.
         */
        FERRISEXP_API void processAllPending_VFSFD_Events();
    };


    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    /**
     * Strip off tailstr from AttrName for an attribute in the context 'c'.
     */
    FERRISEXP_API std::string
    getBaseEAName( std::string tailstr, Context* c, const std::string& attrName );


    class FerrisInternal
    {
    public:
        static void reparentSelectionContext( Context* parent, fh_context NewChildToAdopt, const std::string& rdn );
    };
    

    /*
     * Keeps a collection of every context that exists for debugging memory handle
     * ref counts.
     */
#define DEBUG_CONTEXT_MEMORY_MANAGEMENT 1

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/

    namespace ImplementationDetail
    {
        /**
         * get the --proxy foo:12 string if the user has a proxy configured.
         */
        FERRISEXP_API std::string getCURLProxyCommandLineOption();
    };
    
    FERRISEXP_DLLLOCAL bool isInheritingContext( Context* c );
    
    FERRISEXP_API void ensureEAIndexPluginFactoriesAreLoaded();
    FERRISEXP_API void ensureFulltextIndexPluginFactoriesAreLoaded();

    FERRISEXP_API bool setForceOutOfProcessMetadataOff( bool v );
    
};


#endif // ifndef _ALREADY_INCLUDED_FERRIS_PRIV_H_
