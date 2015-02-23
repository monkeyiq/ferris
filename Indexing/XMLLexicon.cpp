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

    $Id: XMLLexicon.cpp,v 1.3 2010/09/24 21:31:08 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

/**
 * Just stores the data without compression in an XML file.
 *
 * The entire lexicon is loaded into RAM at startup so it should operate
 * very quickly
 */
#include <config.h>

#include "FullTextIndexer.hh"
#include "FullTextIndexer_private.hh"
#include "IndexPrivate.hh"
#include <Configuration_private.hh>
#include <FerrisBackup.hh>
#include <FerrisDOM.hh>

#include <Singleton.h>
#include <Factory.h>
#include <Functor.h>

#include <STLdb4/stldb4.hh>

#include <iomanip>

#include <errno.h>

using namespace std;
using namespace STLdb4;

#define X(str) XStr(str).unicodeForm()

namespace Ferris
{
    namespace FullTextIndex 
    {
        class FERRISEXP_DLLLOCAL Lexicon_XML
            :
            public Lexicon
        {
            typedef Lexicon_XML  _Self;
            typedef Lexicon      _Base;

            bool m_dirty;
            typedef map< string, termid_t > m_lexicon_t;
            m_lexicon_t  m_lexicon;
            m_lexicon_t::iterator m_iter;
            
            void load();
            void save();
            
        protected:

            virtual void setIndex( fh_idx idx );
            virtual std::string getFirstTerm();
            virtual std::string getNextTerm( const std::string& s );

        public:
            
            Lexicon_XML( fh_idx idx = 0 );
            virtual ~Lexicon_XML();

            virtual void insert( const std::string& term, termid_t termid );
            virtual termid_t lookup( const std::string& term );
            virtual void sync();

            static bool reged;
            static bool regedx;
            static string getClassName()
                { return "XML"; }
        };
        bool Lexicon_XML::reged = LexiconFactory::Instance().
        Register( Lexicon_XML::getClassName(),
                  &MakeObject<Lexicon,Lexicon_XML>::Create );
        bool Lexicon_XML::regedx = appendToLexiconClassNames(
            Lexicon_XML::getClassName() );


        static bool rg = LexiconFactory::Instance().
        Register( "XML", &MakeObject<Lexicon,Lexicon_XML>::Create );
        static bool rx = appendToLexiconAliasNames( "XML" );


        Lexicon_XML::Lexicon_XML( fh_idx idx )
            :
            Lexicon( idx ),
            m_dirty( false )
        {
            setFileName( "/lexicon.xml" );
            m_iter = m_lexicon.end();
        }

        void
        Lexicon_XML::load()
        {
            try
            {
                string filename = CleanupURL( m_path_mgr->getBasePath() + "/" + getFileName() );
                fh_context c = Resolve( filename );
                if( toint( getStrAttr( c, "size", "0" )) > 0 )
                {
                    fh_ifstream iss( filename );
                    fh_domdoc    doc  = Ferris::Factory::StreamToDOM( iss );
                    DOMElement*  root = doc->getDocumentElement();
                    
                    for( DOMNode* n = root->getFirstChild(); n ; n = n->getNextSibling() )
                    {
                        if( n->getNodeType() == DOMNode::ELEMENT_NODE )
                        {
                            DOMElement* child = (DOMElement*)n;
                            string sid  = getAttribute( child, "id" );
                            termid_t id = toType<termid_t>(sid);
                            string term = XML::getChildText( child );
                            m_lexicon.insert( make_pair( term, id ));
                        }
                    }

                    //
                    // The following is MUCH slower!
                    //
//                     cerr << "Lexicon_XML::load(x) nl->getLength:" << nl->getLength() << endl;
//                     for( int i=0; i < nl->getLength(); ++i )
//                     {
//                         if( i % 100 == 1 )
//                         {
//                             cerr << "Lexicon_XML::load(x) nl->getLength:" << nl->getLength()
//                                  << " i:" << i << endl;
//                         }
                        
//                         DOMNode* n = nl->item( i );
//                         if( n->getNodeType() == DOMNode::ELEMENT_NODE )
//                         {
//                             DOMElement* child = (DOMElement*)n;
//                             string sid  = getAttribute( child, "id" );
//                             termid_t id = toType<termid_t>(sid);
// //                            string term = getAttribute( child, "term" );
//                             string term = XML::getChildText( child );
//                             m_lexicon.insert( make_pair( term, id ));
//                         }
//                     }
                }
            }
            catch( NoSuchSubContext& e )
            {}
            catch( exception& e )
            {
                cerr << "Lexicon_XML::load() general e:" << e.what() << endl;
                throw;
            }
        }

        void
        Lexicon_XML::save()
        {
            DOMImplementation *impl = Ferris::Factory::getDefaultDOMImpl();
            fh_domdoc    doc = impl->createDocument( 0, X("lexicon"), 0 );
            DOMElement* root = doc->getDocumentElement();
            
            for( m_lexicon_t::iterator li = m_lexicon.begin(); li!=m_lexicon.end(); ++li )
            {
                DOMElement* e = doc->createElement( X("keyval") );
                root->appendChild( e );
                setAttribute( e, "id",   tostr(li->second) );
//                setAttribute( e, "term", li->first );

                DOMText* payload = doc->createTextNode( X(li->first.c_str()));
                e->appendChild( payload );
            }
            fh_stringstream ss = tostream( doc );

            fh_ofstream oss( CleanupURL( m_path_mgr->getBasePath() + "/" + getFileName() ) );
            std::copy( std::istreambuf_iterator<char>(ss),
                       std::istreambuf_iterator<char>(),
                       std::ostreambuf_iterator<char>(oss));
            oss << flush;
        }
        
        
        void
        Lexicon_XML::setIndex( fh_idx idx )
        {
            _Base::setIndex( idx );
            try
            {
                Ferris::Factory::ensureXMLPlatformInitialized();
                load();
            }
            catch( exception& e )
            {
                LG_IDX_D << "Error opening lexicon e:" << e.what() << endl;
                throw;
            }
        }
        
        
        Lexicon_XML::~Lexicon_XML()
        {
            if( m_dirty )
                save();
        }
        
        void
        Lexicon_XML::insert( const std::string& term, termid_t termid )
        {
            m_lexicon[ term ] = termid;
            m_dirty = true;
        }
        
        termid_t
        Lexicon_XML::lookup( const std::string& term )
        {
            m_lexicon_t::iterator li = m_lexicon.find( term );
            if( li != m_lexicon.end() )
                return li->second;
            return 0;
        }
        
        void
        Lexicon_XML::sync()
        {
            if( m_dirty )
                save();
        }

        std::string
        Lexicon_XML::getFirstTerm()
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
        Lexicon_XML::getNextTerm( const std::string& s )
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

