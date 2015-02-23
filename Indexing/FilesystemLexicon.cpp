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

    $Id: FilesystemLexicon.cpp,v 1.4 2010/09/24 21:31:07 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

/**
 * Implementation of lexicon storage using ferris VFS itself.
 * This is probably the most basic lexicon storage plugin.
 */
#include <config.h>

#include "FullTextIndexer.hh"
#include "FullTextIndexer_private.hh"
#include "IndexPrivate.hh"
#include <Configuration_private.hh>
#include <FerrisBackup.hh>

#include <Singleton.h>
#include <Factory.h>
#include <Functor.h>

#include <iomanip>

#include <errno.h>
#include <time.h>

using namespace std;

namespace Ferris
{
    namespace FullTextIndex 
    {
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        const std::string LEXICON_FILENAME = "lexicon";
        
        class Lexicon_Filesystem;
        FERRIS_SMARTPTR( Lexicon_Filesystem, fh_fclex );

        /**
         * Lexicon for storage in a ferris VFS
         */
        class FERRISEXP_DLLLOCAL Lexicon_Filesystem
            :
            public Lexicon
        {
            typedef Lexicon_Filesystem   _Self;
            typedef Lexicon              _Base;

            fh_context theContext;

            /**
             * Playing around with having a 'cache' that is used during insertion/lookup
             * to speed up this lexicon. It seems that having relatively small delays
             * especially during lookup of terms during inversion of a file causes huge
             * time delays overall.
             */
            typedef map< std::string, termid_t > cache_t;
            cache_t cache;
            
        protected:

            virtual void setIndex( fh_idx idx );
            virtual std::string getFirstTerm();
            virtual std::string getNextTerm( const std::string& s );

        public:
            
            Lexicon_Filesystem( fh_idx idx = 0 )
                : Lexicon( idx )
                {}
            virtual ~Lexicon_Filesystem()
                {}
            
            virtual void insert( const std::string& term, termid_t termid );
            virtual termid_t lookup( const std::string& term );
            virtual void sync()
                {}
            

            
            static bool reged;
            static bool regedx;
            static string getClassName()
                { return "Filesystem"; }
        };
        bool Lexicon_Filesystem::reged = LexiconFactory::Instance().
        Register( Lexicon_Filesystem::getClassName(),
                  &MakeObject<Lexicon,Lexicon_Filesystem>::Create );
        bool Lexicon_Filesystem::regedx = appendToLexiconClassNames(
            Lexicon_Filesystem::getClassName() );

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        void
        Lexicon_Filesystem::setIndex( fh_idx idx )
        {
            _Base::setIndex( idx );

            string filename = m_path_mgr->getBasePath() + "/" + LEXICON_FILENAME;
            LG_IDX_D << "Lexicon_Filesystem::setIndex() mkdir filename:" << filename << endl;
            theContext = Shell::touch( filename, true, true );
        }
        
        void
        Lexicon_Filesystem::insert( const std::string& term, termid_t termid )
        {
            fh_context c = Shell::CreateFile( theContext, term );
            setStrAttr( c, "content", tostr( termid ));
            cache[ term ] = termid;
        }
        
        termid_t
        Lexicon_Filesystem::lookup( const std::string& term )
        {
            // check cache
            {
                cache_t::iterator ci = cache.find( term );
                if( ci != cache.end() )
                {
                    return ci->second;
                }
            }
            
            Context::iterator ci = theContext->find( term );
            if( ci != theContext->end() )
            {
                termid_t ret = toType<termid_t>( getStrAttr( *ci, "content", "", true, true ));

                cache[ term ] = ret;
                return ret;
            }
            return 0;
        }


        std::string
        Lexicon_Filesystem::getFirstTerm()
        {
            Context::iterator ci = theContext->begin();
            if( ci == theContext->end() )
                return "";
            return (*ci)->getDirName();
        }

        
        std::string
        Lexicon_Filesystem::getNextTerm( const std::string& s )
        {
            Context::iterator ci = theContext->find( s );
            if( ci != theContext->end() )
                ++ci;
            if( ci == theContext->end() )
                return "";
            return (*ci)->getDirName();
        }
    };
};
