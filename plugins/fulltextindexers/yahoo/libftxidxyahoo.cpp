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

    $Id: libftxidxyahoo.cpp,v 1.3 2008/12/19 21:30:14 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/FullTextIndexerMetaInterface.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>

#include <string>
using namespace std;

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


namespace Ferris
{
    namespace FullTextIndex 
    {
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_DLLLOCAL FullTextIndexerYahoo
            :
            public MetaFullTextIndexerInterface
        {
            typedef map< int, string > m_docIDs_t;
            m_docIDs_t m_docIDs;
            typedef map< string, int > m_docIDs_Reverse_t;
            m_docIDs_Reverse_t m_docIDs_Reverse;
            
        protected:

            
            virtual void Setup();
            virtual void CreateIndexBeforeConfig( fh_context c,
                                                  bool caseSensitive,
                                                  bool dropStopWords,
                                                  StemMode stemMode,
                                                  const std::string& lex_class,
                                                  fh_context md );
            virtual void CreateIndex( fh_context c,
                                      bool caseSensitive,
                                      bool dropStopWords,
                                      StemMode stemMode,
                                      const std::string& lex_class,
                                      fh_context md );
            virtual void CommonConstruction();
            
            virtual void addToIndex( fh_context c,
                                     fh_docindexer di );

            fh_domdoc performQuery( const std::string q );
            docNumSet_t& addAllMatches( fh_domdoc dom,  docNumSet_t& output );
            
            virtual docNumSet_t& addAllDocumentsMatchingTerm(
                const std::string& term,
                docNumSet_t& output,
                int limit );
            virtual
            docNumSet_t&
            ExecuteWebFullTextQuery( const std::string& queryString,
                                     docNumSet_t& docnums,
                                     int limit );
            
            virtual std::string resolveDocumentID( docid_t );
            int getDocID( const std::string earl );

        public:

            FullTextIndexerYahoo();
            virtual ~FullTextIndexerYahoo();
        };

        

        /************************************************************/
        /************************************************************/
        /************************************************************/

        FullTextIndexerYahoo::FullTextIndexerYahoo()
        {
        }

        FullTextIndexerYahoo::~FullTextIndexerYahoo()
        {
        }

        
        
        void
        FullTextIndexerYahoo::Setup()
        {
        }

        void
        FullTextIndexerYahoo::CreateIndexBeforeConfig( fh_context c,
                                                          bool caseSensitive,
                                                          bool dropStopWords,
                                                          StemMode stemMode,
                                                          const std::string& lex_class,
                                                          fh_context md )
        {
        }
        
        
        void
        FullTextIndexerYahoo::CreateIndex( fh_context c,
                                           bool caseSensitive,
                                           bool dropStopWords,
                                           StemMode stemMode,
                                           const std::string& lex_class,
                                           fh_context md )
        {
        }

        void
        FullTextIndexerYahoo::CommonConstruction()
        {
        }


        void
        FullTextIndexerYahoo::addToIndex( fh_context c, fh_docindexer di )
        {
            stringstream ss;
            ss << "Sorry, you can not explicitly add new files to a yahoo! index." << endl;
            Throw_IndexException( ss.str(), 0 );
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        std::string
        FullTextIndexerYahoo::resolveDocumentID( docid_t id )
        {
            return m_docIDs[ id ];
        }

        int
        FullTextIndexerYahoo::getDocID( const std::string earl )
        {
            m_docIDs_Reverse_t::iterator ri = m_docIDs_Reverse.find( earl );
            if( ri != m_docIDs_Reverse.end() )
            {
                return ri->second;
            }
            
            static int newID = 0;
            ++newID;
            m_docIDs[ newID ] = earl;
            m_docIDs_Reverse[ earl ] = newID;
            return newID;
        }

        docNumSet_t&
        FullTextIndexerYahoo::addAllMatches( fh_domdoc dom, 
                                             docNumSet_t& output )
        {
            DOMElement* root = dom->getDocumentElement();
            typedef std::list< DOMElement* > el_t;
            el_t el;

            el = XML::getAllChildrenElements( root, "Result", el, true );

            LG_IDX_D << "addAllMatches() el.sz:" << el.size()
                     << " root:" << tostr(root->getNodeName())
                     << endl;
            for( el_t::iterator ei = el.begin(); ei != el.end(); ++ei )
            {
                DOMElement* n = *ei;
                DOMElement* title_e   = XML::getChildElement( n, "Title" );
                DOMElement* summary_e = XML::getChildElement( n, "Summary" );
                DOMElement* url_e     = XML::getChildElement( n, "Url" );

                string earl = XML::getChildText( url_e );
                LG_IDX_D << "addAllMatches(iter) earl:" << earl
                         << endl;
                
                int docid = getDocID( earl );
                output.insert( docid );
            }
            return output;
        }
        
        fh_domdoc
        FullTextIndexerYahoo::performQuery( const std::string q )
        {
            fh_stringstream datass;
            
            stringstream cmdss;
            cmdss << "fcat 'http://api.search.yahoo.com/WebSearchService/V1/webSearch?"
                  << "query=" << q
                  << "&appid=libferris'";
            fh_runner r = new Runner();
            r->setSpawnFlags(
                GSpawnFlags(
                    G_SPAWN_SEARCH_PATH |
                    G_SPAWN_LEAVE_DESCRIPTORS_OPEN |
                    G_SPAWN_STDERR_TO_DEV_NULL |
                    r->getSpawnFlags()));
            LG_IDX_D << "cmd:" << cmdss.str() << endl;
            r->setCommandLine( tostr(cmdss) );
            r->Run();

            fh_istream stdoutss = r->getStdOut();
            copy( istreambuf_iterator<char>(stdoutss),
                  istreambuf_iterator<char>(),
                  ostreambuf_iterator<char>(datass));
            gint e = r->getExitStatus();

            fh_domdoc ret = Ferris::Factory::StringToDOM( tostr(datass) );
            return ret;
        }
        
        
        docNumSet_t&
        FullTextIndexerYahoo::addAllDocumentsMatchingTerm(
            const std::string& term_const,
            docNumSet_t& output,
            int limit )
        {
            fh_domdoc dom = performQuery( term_const );
            addAllMatches( dom, output );
            return output;
        }
        
        docNumSet_t&
        FullTextIndexerYahoo::ExecuteWebFullTextQuery( const std::string& queryString,
                                                       docNumSet_t& output,
                                                       int limit )
        {
            fh_domdoc dom = performQuery( queryString );
            addAllMatches( dom, output );
            return output;
        }
        


        
        
        /**************************************************/
        /**************************************************/

        
    };
};

extern "C"
{
    FERRISEXP_API Ferris::FullTextIndex::MetaFullTextIndexerInterface* Create()
    {
        return new Ferris::FullTextIndex::FullTextIndexerYahoo();
    }
};
