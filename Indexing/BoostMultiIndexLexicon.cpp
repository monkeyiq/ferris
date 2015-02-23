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

    $Id: BoostMultiIndexLexicon.cpp,v 1.3 2010/09/24 21:31:07 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

/**
 * uses a boost::multi_index to store both a forward and reverse lexicon
 */
#include <config.h>

#include "FullTextIndexer.hh"
#include "FullTextIndexer_private.hh"
#include "IndexPrivate.hh"
#include <Configuration_private.hh>
//#include <FerrisBackup.hh>

// #if !defined(NDEBUG)
// #define BOOST_MULTI_INDEX_ENABLE_INVARIANT_CHECKING
// #define BOOST_MULTI_INDEX_ENABLE_SAFE_MODE
// #endif

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index/ordered_index.hpp>
#include <boost/tuple/tuple_io.hpp>

#include <Singleton.h>
#include <Factory.h>
#include <Functor.h>

#include <iomanip>
#include <errno.h>

using namespace std;
using namespace boost;
using namespace boost::multi_index;

namespace boost {
    namespace serialization {

        template<class Archive, class FirstT, class SecondT >
        void serialize(Archive & ar, std::pair< FirstT, SecondT >& p, const unsigned int version)
        {
            ar & p.first;
            ar & p.second;
        }

    } // namespace serialization
} // namespace boost

namespace Ferris
{
    struct from{};
    struct to{};
    template<typename FromType,typename ToType>
    struct bidirectional_map
    {
        typedef bidirectional_map _Self;
        
       typedef std::pair<FromType,ToType> value_type;
        typedef multi_index_container<
            value_type,
            indexed_by<
                ordered_unique<
                    tag<from>,member<value_type,FromType,&value_type::first> >,
                ordered_unique<
                    tag<to>,  member<value_type,ToType,&value_type::second> >
            >
        > type;
    };
    
    
    namespace FullTextIndex 
    {
        class FERRISEXP_DLLLOCAL Lexicon_BoostMultiIndex
            :
            public Lexicon
        {
            typedef Lexicon_BoostMultiIndex  _Self;
            typedef Lexicon      _Base;

            bool m_dirty;
            
            typedef bidirectional_map<string, termid_t>::type m_lexicon_t;
            m_lexicon_t  m_lexicon;
            m_lexicon_t::iterator m_iter;
            
            void load();
            void save();
            
        protected:

            virtual void setIndex( fh_idx idx );
            virtual std::string getFirstTerm();
            virtual std::string getNextTerm( const std::string& s );

        public:
            
            Lexicon_BoostMultiIndex( fh_idx idx = 0 );
            virtual ~Lexicon_BoostMultiIndex();

            virtual void insert( const std::string& term, termid_t termid );
            virtual termid_t lookup( const std::string& term );
            virtual void sync();

            static bool reged;
            static bool regedx;
            static string getClassName()
                { return "boost-multi-index"; }
        };
        bool Lexicon_BoostMultiIndex::reged = LexiconFactory::Instance().
        Register( Lexicon_BoostMultiIndex::getClassName(),
                  &MakeObject<Lexicon,Lexicon_BoostMultiIndex>::Create );
        bool Lexicon_BoostMultiIndex::regedx = appendToLexiconClassNames(
            Lexicon_BoostMultiIndex::getClassName() );


        static bool rg = LexiconFactory::Instance().
        Register( "boost-multi-index", &MakeObject<Lexicon,Lexicon_BoostMultiIndex>::Create );
        static bool rx = appendToLexiconAliasNames( "boost-multi-index" );


        Lexicon_BoostMultiIndex::Lexicon_BoostMultiIndex( fh_idx idx )
            :
            Lexicon( idx ),
            m_dirty( false )
        {
            setFileName( "/lexicon.boost-multi-index" );
            m_iter = m_lexicon.end();
        }

        void
        Lexicon_BoostMultiIndex::load()
        {
            string filename = CleanupURL( m_path_mgr->getBasePath() + "/" + getFileName() );
            std::ifstream ifs(filename.c_str());
            if(ifs) {
                boost::archive::binary_iarchive ia(ifs);
                ia >> m_lexicon ;
            }
        }

        void
        Lexicon_BoostMultiIndex::save()
        {
            string filename = CleanupURL( m_path_mgr->getBasePath() + "/" + getFileName() );
            std::ofstream ofs( filename.c_str() );
            boost::archive::binary_oarchive oa(ofs);
            const m_lexicon_t& work_around = m_lexicon; 
            oa << work_around;
        }
        
        
        void
        Lexicon_BoostMultiIndex::setIndex( fh_idx idx )
        {
            _Base::setIndex( idx );
            try
            {
                load();
            }
            catch( std::exception& e )
            {
                LG_IDX_D << "Error opening lexicon e:" << e.what() << endl;
                throw;
            }
        }
        
        
        Lexicon_BoostMultiIndex::~Lexicon_BoostMultiIndex()
        {
            if( m_dirty )
                save();
        }
        
        void
        Lexicon_BoostMultiIndex::insert( const std::string& term, termid_t termid )
        {
            m_lexicon.insert( m_lexicon_t::value_type( term, termid ) );
            m_dirty = true;
        }
        
        termid_t
        Lexicon_BoostMultiIndex::lookup( const std::string& term )
        {
            m_lexicon_t::iterator li = m_lexicon.find( term );
            if( li != m_lexicon.end() )
                return li->second;
            return 0;
        }
        
        void
        Lexicon_BoostMultiIndex::sync()
        {
            if( m_dirty )
                save();
        }

        std::string
        Lexicon_BoostMultiIndex::getFirstTerm()
        {
            if( m_lexicon.empty() )
            {
                m_iter = m_lexicon.end();
                return "";
            }
            
            m_iter = m_lexicon.begin();
            return getNextTerm("");
        }
        
        std::string
        Lexicon_BoostMultiIndex::getNextTerm( const std::string& s )
        {
            if( m_iter == m_lexicon.end() )
                return "";

            string v = m_iter->first;
            ++m_iter;
            return v;
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
    };
};

