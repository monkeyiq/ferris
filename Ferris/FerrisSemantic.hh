/******************************************************************************
*******************************************************************************
*******************************************************************************

    soprano RDF higher level semantic functions.  

    Copyright (C) 2003+ Ben Martin

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

    $Id: FerrisSemantic.hh,v 1.4 2011/04/08 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_SEMANTIC_H_
#define _ALREADY_INCLUDED_FERRIS_SEMANTIC_H_

#include <Ferris/TypeDecl.hh>
#include <Ferris/FerrisRDFCore.hh>

#ifdef emit
#undef emit
#endif

#ifdef signal
#undef signal
#endif


namespace boost {
    namespace serialization {
        class access;
    }
}


namespace Ferris
{
    namespace Semantic
    {
        using namespace ::Ferris::RDFCore;

        namespace Wordnet
        {
            FERRISEXP_API extern std::string RDFSCHEMA;
            FERRISEXP_API extern std::string WNPFX;

            using ::Ferris::RDFCore::nodelist_t;
//            typedef std::list< fh_node > nodelist_t;

            FERRISEXP_API fh_node& hasWord();
            FERRISEXP_API fh_node& hasSynSet();
            FERRISEXP_API fh_node& NounSynSet();
            FERRISEXP_API fh_node& Type();
            FERRISEXP_API fh_node& hasGloss();
            FERRISEXP_API fh_node& hyponymOf();
            FERRISEXP_API fh_node& partMeronymOf();
            
            FERRISEXP_API nodelist_t& getSynSet( fh_model m, nodelist_t& ret, const std::string& word );
            FERRISEXP_API std::string getNodeID( fh_node n );
            FERRISEXP_API std::string getWordNodeAsString( fh_node n );

            FERRISEXP_API fh_model getWordnetModel();
        };
        

        FERRISEXP_API std::string getAttrPrefix();
        FERRISEXP_API std::string getPredicateURI( const std::string& rdn );
        FERRISEXP_API std::string stripPredicateURIPrefix( const std::string& pred );

        /**
         * After a file is moved/copied outside of libferris this call can attach
         * EA which is stored in myrdf.db to the new version of the context.
         * eg. mv foo foo1; myrdfSmush( Resolve( foo1 ), foo );
         */
        FERRISEXP_API void myrdfSmush( fh_context existingc, const std::string& oldpath, bool unifyUUIDNodes = false );
        FERRISEXP_API void myrdfSmush( fh_context existingc, fh_context oldc, bool unifyUUIDNodes = false );

        /**
         * Merge the myrdf metadata for the given list of URLs making each URL
         * see the union of this metadata. If an EA is bound for two URLs
         * each URL will see its old value. So this really only imports the
         * EAs which are not bound for each respective URL in the set from all
         * the others in the set.
         */
        FERRISEXP_API void myrdfSmushUnion( const stringlist_t& srcs );

        /**
         * Filesystem objects have a UUID node which has metadata attached to it.
         * Conceptually it is something like this
         *
         * file-url uuid-pred uuidnode
         * uuidnode soprano-ea-pred SopranoEASubjectNodeX
         * SopranoEASubjectNodeX eaname eavalue
         *
         * Get the uuidnode with ensureUUIDNode().
         * Get the SopranoEASubjectNodeX to get all the out-of-band EA stored in myrdf
         * using ensureSopranoEASubjectNode()
         * 
         * The uuid-pred link can be made implicitly sometimes by libferris so its
         * best to call tryToGetUUIDNode( fh_context ) to find the UUID node
         * associated with a context if one exists.
         */
        FERRISEXP_API ::Ferris::RDFCore::fh_node tryToGetUUIDNode( fh_context c );

        /**
         * Like tryToGetUUIDNode() but if no such link exists it is created
         * automatically.
         */
        FERRISEXP_API ::Ferris::RDFCore::fh_node ensureUUIDNode( fh_context c );
        FERRISEXP_API ::Ferris::RDFCore::fh_node ensureUUIDNode( ::Ferris::RDFCore::fh_model m, fh_context c );

        /**
         * Get the last time_t any of the RDF data relating to this uuidnode was
         * changed.
         */
        FERRISEXP_API time_t getUUIDNodeModificationTime( ::Ferris::RDFCore::fh_node n );

        /**
         * Record the time_t that an update for any RDF data relatiing to this
         * uuidnode has changed.
         */
        FERRISEXP_API void setUUIDNodeModificationTime( ::Ferris::RDFCore::fh_node n,
                                                        time_t t = 0 );
        FERRISEXP_API void setSopranoEASubjectNode( ::Ferris::RDFCore::fh_node n,
                                                    time_t t = 0 );
        
        /**
         * Try to get the subject node for a context where EAName EAValue predicate object
         * pairings are stored.
         *
         * @see ensureSopranoEASubjectNode()
         */
        FERRISEXP_API ::Ferris::RDFCore::fh_node tryToGetSopranoEASubjectNode( fh_context c );
        FERRISEXP_API ::Ferris::RDFCore::fh_node ensureSopranoEASubjectNode( fh_context c );
        
        /**
         * This is mainly for applications like format converters which need to
         * setup uuid-pred links themselves.
         * Applications should use tryToGetUUIDNode() and ensureUUIDNode().
         */
        FERRISEXP_API ::Ferris::RDFCore::fh_node& uuidPredNode();
        /**
         * Used for creating a quick
         * smushsetid-filename smushCacheUUIDPredNode() uuidnode
         * cache for fast smushing on filename.
         */
        FERRISEXP_API ::Ferris::RDFCore::fh_node& smushCacheUUIDPredNode();
        

        /**
         * This is mainly for applications like format converters which need to
         * setup uuid-pred links themselves.
         * Applications should use tryToGetUUIDNode() and ensureUUIDNode().
         */
        FERRISEXP_API ::Ferris::RDFCore::fh_node& uuidOutOfBandPredNode();
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        
        class FERRISEXP_API SmushSet
            :
            public Handlable
        {
        public:
            virtual ~SmushSet();

            const std::string& getName() const;
            void setName( const std::string& n );

            typedef std::map< std::string, fh_regex > m_smushes_t;
            const m_smushes_t& getSmushes() const;
            const std::string& getRegexString();
            fh_rex getRegex();
            void setSmushes( m_smushes_t& v );
            
        private:
            std::string m_name;
            m_smushes_t m_smushes;
            std::string m_regexString;
            fh_rex      m_rex;
            friend class TreeSmushing;
            fh_TreeSmushing m_treeSmushing;
            SmushSet( fh_TreeSmushing p, const std::string& name );
            
            friend class boost::serialization::access;
        };

        FERRISEXP_API fh_TreeSmushing getDefaultImplicitTreeSmushing();
        
        class FERRISEXP_API TreeSmushing
            :
            public Handlable
        {
        public:
            TreeSmushing();
            virtual ~TreeSmushing();

            typedef std::map< std::string, fh_SmushSet > m_smushSets_t;

            const m_smushSets_t& getAll() const;
            fh_SmushSet getSmush( const std::string& n ) const;
            void swap( fh_TreeSmushing other );
            fh_SmushSet newSmush( const std::string& name );
            void sync();

            template< class ArchiveClassT >
            void sync( ArchiveClassT& archive );

            
        private:
            bool m_shouldSave;
            m_smushSets_t m_smushSets;

            TreeSmushing( bool shouldSave );
            friend fh_TreeSmushing getDefaultImplicitTreeSmushing();

            static std::string getBoostSerializePath();
            
            friend class SmushSet;
            void updateSmushName( const std::string& oldname, const std::string& newname );

            friend class boost::serialization::access;
            template<class Archive>
            void serialize(Archive & ar, const unsigned int version)
                {
                    ar & m_smushSets;
                }
            template<class Archive> void load( Archive & ar );
        };
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    };
};

#endif
