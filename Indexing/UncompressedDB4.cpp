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

    $Id: UncompressedDB4.cpp,v 1.4 2010/09/24 21:31:08 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

/**
 * Just stores the data without compression in a db4 file.
 *
 * Should be the simplest codepath and maybe the fastest for very fast disks
 * with large in memory caches.
 */

#include "FullTextIndexer.hh"
#include "FullTextIndexer_private.hh"
#include "IndexPrivate.hh"
#include <Configuration_private.hh>
#include <FerrisBackup.hh>

#include <Singleton.h>
#include <Factory.h>
#include <Functor.h>

#include <STLdb4/stldb4.hh>

#include <iomanip>

#include <errno.h>

using namespace std;
using namespace STLdb4;


namespace Ferris
{
    namespace FullTextIndex 
    {
        class FERRISEXP_DLLLOCAL Lexicon_Uncompressed
            :
            public Lexicon
        {
            typedef Lexicon_Uncompressed _Self;
            typedef Lexicon              _Base;
            
            fh_database m_db;
            Database::iterator m_iter;

        protected:

            virtual void setIndex( fh_idx idx );
            virtual std::string getFirstTerm();
            virtual std::string getNextTerm( const std::string& s );

        public:
            
            Lexicon_Uncompressed( fh_idx idx = 0 );
            virtual ~Lexicon_Uncompressed();

            virtual void insert( const std::string& term, termid_t termid );
            virtual termid_t lookup( const std::string& term );
            virtual void sync();

            static bool reged;
            static bool regedx;
            static string getClassName()
                { return "Uncompressed (db4 hash)"; }
        };
        bool Lexicon_Uncompressed::reged = LexiconFactory::Instance().
        Register( Lexicon_Uncompressed::getClassName(),
                  &MakeObject<Lexicon,Lexicon_Uncompressed>::Create );
        bool Lexicon_Uncompressed::regedx = appendToLexiconClassNames(
            Lexicon_Uncompressed::getClassName() );


        static bool rg = LexiconFactory::Instance().
        Register( "Uncompressed", &MakeObject<Lexicon,Lexicon_Uncompressed>::Create );
        static bool rx = appendToLexiconAliasNames( "Uncompressed" );


        Lexicon_Uncompressed::Lexicon_Uncompressed( fh_idx idx )
            :
            Lexicon( idx ),
            m_db( 0 )
        {
            setFileName( "/lexicon.db" );
        }

        void
        Lexicon_Uncompressed::setIndex( fh_idx idx )
        {
            _Base::setIndex( idx );
            try
            {
                string filename = CleanupURL( m_path_mgr->getBasePath() + "/" + getFileName() );
                LG_IDX_D << "Lexicon_Uncompressed::setIndex() filename:" << filename << endl;
                m_db = new Database( DB_HASH, filename );
            }
            catch( exception& e )
            {
                LG_IDX_W << "Error opening lexicon e:" << e.what() << endl;
                throw;
            }
        }
        
        
        Lexicon_Uncompressed::~Lexicon_Uncompressed()
        {
            if( m_db )
                m_db->sync();
        }
        
        void
        Lexicon_Uncompressed::insert( const std::string& term, termid_t termid )
        {
            LG_IDX_D << "Lexicon_Uncompressed::insert() term:" << term << " tid:" << termid
                     << " db:" << toVoid( GetImpl( m_db )) << endl;
            
            m_db[ term ] = tostr( termid );
        }
        
        termid_t
        Lexicon_Uncompressed::lookup( const std::string& term )
        {
            string s = tostr( 0 );
            Database::iterator di = m_db->find( term );
            if( di != m_db->end() )
                di.getValue( s );
            return toType< termid_t >( s );
        }
        
        void
        Lexicon_Uncompressed::sync()
        {
            if( m_db )
                m_db->sync();
        }

        std::string
        Lexicon_Uncompressed::getFirstTerm()
        {
            LG_IDX_D << "Lexicon_Uncompressed::getFirstTerm(t)" << endl;
            LG_IDX_D << "Lexicon_Uncompressed::getFirstTerm(b==e)"
                     << (m_db->begin() == m_db->end()) << endl;
            if( m_db->begin() == m_db->end() )
            {
                m_iter = m_db->end();
                return "";
            }
            
            m_iter = m_db->begin();
            return getNextTerm("");
        }
        
        std::string
        Lexicon_Uncompressed::getNextTerm( const std::string& s )
        {
            if( m_iter == m_db->end() )
                return "";

            string v = "";
            m_iter.getKey( v );
            ++m_iter;
            return v;
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_DLLLOCAL ReverseLexicon_Uncompressed
            :
            public ReverseLexicon
        {
            fh_database getDB();
            fh_database m_db;
            Database::iterator m_iter;
            
        protected:

            virtual termid_t getFirstTerm();
            virtual termid_t getNextTerm( termid_t t );
            
        public:
            ReverseLexicon_Uncompressed()
                :
                m_db( 0 )
                {
                    setFileName("uncompressed.rlexicon");
                }
            
            virtual void insert( const std::string& s, termid_t id );
            virtual std::string lookup( termid_t id );
            virtual bool exists( termid_t id );
            virtual void sync()
                {
                    if( m_db )
                        getDB()->sync();
                }

            static bool reged;
            static bool regedx;
            static string getClassName()
                { return "Uncompressed (db4 hash)"; }
        };
        bool ReverseLexicon_Uncompressed::reged = ReverseLexiconFactory::Instance().
        Register( ReverseLexicon_Uncompressed::getClassName(),
                  &MakeObject<ReverseLexicon,ReverseLexicon_Uncompressed>::Create );
        bool ReverseLexicon_Uncompressed::regedx = appendToReverseLexiconClassNames(
            ReverseLexicon_Uncompressed::getClassName() );

        static bool rlx_reged = ReverseLexiconFactory::Instance().
        Register( "Uncompressed",
                  &MakeObject<ReverseLexicon,ReverseLexicon_Uncompressed>::Create );
        static bool rlx_regedx = appendToReverseLexiconClassNames( "Uncompressed" );
        
        
        fh_database
        ReverseLexicon_Uncompressed::getDB()
        {
            if( !m_db )
            {
                string filename = CleanupURL( m_path_mgr->getBasePath() + "/" + getFileName() );
                LG_IDX_D << "ReverseLexicon_Uncompressed::getDB() fn:" << filename << endl;
                
                m_db = new Database( DB_HASH, filename );
            }
            return m_db;
        }
        
        
        void
        ReverseLexicon_Uncompressed::insert( const std::string& s, termid_t id )
        {
            fh_database db = getDB();
            LG_IDX_D << "ReverseLexicon_Uncompressed::insert() s:" << s << " id:" << id << endl;
            db[ tostr(id) ] = s;
        }
        
        std::string
        ReverseLexicon_Uncompressed::lookup( termid_t id )
        {
            fh_database db = getDB();
            return db[ tostr(id) ];
        }
        
        bool
        ReverseLexicon_Uncompressed::exists( termid_t id )
        {
            fh_database db = getDB();
            return db->find( tostr(id) ) != db->end();
        }
        
        
        termid_t
        ReverseLexicon_Uncompressed::getFirstTerm()
        {
            fh_database db = getDB();

            if( db->begin() == db->end() )
            {
                m_iter = db->end();
                return 0;
            }
            
            m_iter = db->begin();
            return getNextTerm( 0 );
        }
        
        termid_t
        ReverseLexicon_Uncompressed::getNextTerm( termid_t t )
        {
            fh_database db = getDB();
            if( m_iter == db->end() )
                return 0;

            string v;
            m_iter.getKey( v );
            ++m_iter;
            return toType<termid_t>(v);
        }
        
        
    };
};

