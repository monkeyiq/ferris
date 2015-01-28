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

    $Id: libftxidxexternal.cpp,v 1.3 2008/12/19 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/FullTextIndexerMetaInterface.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/Runner.hh>
#include <Ferris/FerrisDOM.hh>
#include <Ferris/FullTextIndexer_private.hh>
#include <Ferris/EAIndexerSQLCommon_private.hh>
#include <Ferris/FullTextIndexerSyntheticDocID_private.hh>

#include <string>
using namespace std;


/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


namespace Ferris
{
    namespace FullTextIndex 
    {
        static const char* CFG_IDX_SCRIPT_PATH_K   = "cfg-idx-script-path";
        static const char* CFG_IDX_SCRIPT_PATH_DEF = "";
        
        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
        class FERRISEXP_API FullTextIndexerExternal
            :
            public FullTextIndexerSyntheticDocID
        {
//             struct ContainerItem
//             {
//                 int docid;
//                 string earl;
//                 ContainerItem( int docid = 0, const std::string& earl = "" )
//                     :
//                     docid( docid ),
//                     earl( earl )
//                     {
//                     }
//                 inline int getDocID() const
//                     {
//                         return docid;
//                     }
//                 inline const std::string& getEarl() const
//                     {
//                         return earl;
//                     }
                
//             };
//             struct ITEMS_BY_DOCID {};
//             struct ITEMS_BY_EARL {};
            
//             typedef boost::multi_index::multi_index_container<
//                 ContainerItem,
//                 boost::multi_index::indexed_by<
                
//                 boost::multi_index::ordered_unique<
//                 boost::multi_index::tag<ITEMS_BY_EARL>,
//                 boost::multi_index::const_mem_fun<ContainerItem,
//                                                   const std::string&,&ContainerItem::getEarl> >,
                
//                 boost::multi_index::hashed_unique<
//                 boost::multi_index::tag<ITEMS_BY_DOCID>,
//                 boost::multi_index::const_mem_fun<ContainerItem,
//                                                   int,&ContainerItem::getDocID> >
//             > 
//             > Items_t;
//             Items_t Items;
//             typedef Items_t::index<ITEMS_BY_DOCID>::type Items_By_DocID_t;
//             typedef Items_t::index<ITEMS_BY_EARL>::type  Items_By_Earl_t;
            
            string m_scriptPath;
            
//             typedef map< int, string > m_docIDs_t;
//             m_docIDs_t m_docIDs;
//             typedef map< string, int > m_docIDs_Reverse_t;
//             m_docIDs_Reverse_t m_docIDs_Reverse;
            
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

            fh_domdoc performQuery( const std::string q, int limit );
            docNumSet_t&
            addAllMatches( fh_domdoc dom,  docNumSet_t& output );
            
            virtual docNumSet_t& addAllDocumentsMatchingTerm(
                const std::string& term,
                docNumSet_t& output,
                int limit );

            virtual
            docNumSet_t&
            ExecuteExternalFullTextQuery( const std::string& queryString,
                                          docNumSet_t& docnums,
                                          int limit );
            
            virtual std::string resolveDocumentID( docid_t );
            int getDocID( const std::string earl );

        public:

            FullTextIndexerExternal();
            virtual ~FullTextIndexerExternal();
        };

        

        /************************************************************/
        /************************************************************/
        /************************************************************/

        FullTextIndexerExternal::FullTextIndexerExternal()
            :
            m_scriptPath( "" )
        {
        }

        FullTextIndexerExternal::~FullTextIndexerExternal()
        {
        }

        
        
        void
        FullTextIndexerExternal::Setup()
        {
            m_scriptPath = this->getConfig( CFG_IDX_SCRIPT_PATH_K,
                                            CFG_IDX_SCRIPT_PATH_DEF,
                                            true );
        }

        void
        FullTextIndexerExternal::CreateIndexBeforeConfig( fh_context c,
                                                          bool caseSensitive,
                                                          bool dropStopWords,
                                                          StemMode stemMode,
                                                          const std::string& lex_class,
                                                          fh_context md )
        {
        }
        
        
        void
        FullTextIndexerExternal::CreateIndex( fh_context c,
                                           bool caseSensitive,
                                           bool dropStopWords,
                                           StemMode stemMode,
                                           const std::string& lex_class,
                                           fh_context md )
        {
            m_scriptPath = getStrSubCtx( md, "script-path", CFG_IDX_SCRIPT_PATH_DEF );

            // Make sure it actually exists!
            fh_context t = Resolve( m_scriptPath );
            
            setConfig( CFG_IDX_SCRIPT_PATH_K, m_scriptPath );
            
        }

        void
        FullTextIndexerExternal::CommonConstruction()
        {
        }


        void
        FullTextIndexerExternal::addToIndex( fh_context c, fh_docindexer di )
        {
            stringstream ss;
            ss << "Sorry, you can not explicitly add new files to a external index." << endl;
            Throw_IndexException( ss.str(), 0 );
        }

        /********************************************************************/
        /********************************************************************/
        /********************************************************************/
        
//         std::string
//         FullTextIndexerExternal::resolveDocumentID( docid_t id )
//         {
//             Items_By_DocID_t::iterator diter = Items.get<ITEMS_BY_DOCID>().find( id );
//             if( diter == Items.get<ITEMS_BY_DOCID>().end() )
//             {
//                 LG_IDX_W << "resolveDocumentID() id:" << id << " has no URL!" << endl;
//                 return "";
//             }
//             return diter->earl;
//         }

//         int
//         FullTextIndexerExternal::getDocID( const std::string earl )
//         {
//             Items_By_Earl_t::iterator eiter
//                 = Items.get<ITEMS_BY_EARL>().find( earl );
//             if( eiter != Items.get<ITEMS_BY_EARL>().end() )
//             {
//                 return eiter->docid;
//             }
            
//             static int newID = 0;
//             ++newID;
//             Items.insert( ContainerItem( newID, earl ) );
//             return newID;
//         }

        docNumSet_t&
        FullTextIndexerExternal::addAllMatches( fh_domdoc dom, 
                                                docNumSet_t& output )
        {
            DOMElement* root = dom->getDocumentElement();
            typedef std::list< DOMElement* > el_t;
            el_t el;

            el = XML::getAllChildrenElements( root, "context", el, true );

            LG_IDX_D << "addAllMatches() el.sz:" << el.size()
                     << " root:" << tostr(root->getNodeName())
                     << endl;
            for( el_t::iterator ei = el.begin(); ei != el.end(); ++ei )
            {
                DOMElement* n = *ei;

                string earl = getAttribute( n, "url" );
                LG_IDX_D << "addAllMatches(iter) earl:" << earl
                         << endl;
                
                int docid = getDocID( earl );
                output.insert( docid );
            }
            return output;
        }
        
        fh_domdoc
        FullTextIndexerExternal::performQuery( const std::string q, int limit )
        {
            fh_stringstream datass;
            
            stringstream cmdss;
            cmdss << m_scriptPath << " "
                  << " \"" << q << "\" ";
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

            LG_IDX_D << "Have data:" << tostr(datass) << endl;
            fh_domdoc ret = Ferris::Factory::StringToDOM( tostr(datass) );
            return ret;
        }
        
        
        docNumSet_t&
        FullTextIndexerExternal::addAllDocumentsMatchingTerm(
            const std::string& term_const,
            docNumSet_t& output,
            int limit )
        {
            fh_domdoc dom = performQuery( term_const, limit );
            addAllMatches( dom, output );
            return output;
        }
        
        docNumSet_t&
        FullTextIndexerExternal::ExecuteExternalFullTextQuery( const std::string& queryString,
                                                               docNumSet_t& output,
                                                               int limit )
        {
            LG_IDX_D << "ExecuteExternalFullTextQuery() q-->" << queryString << "<--" << endl;
            fh_domdoc dom = performQuery( queryString, limit );
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
        return new Ferris::FullTextIndex::FullTextIndexerExternal();
    }
};
