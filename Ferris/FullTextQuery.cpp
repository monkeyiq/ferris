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

    $Id: FullTextQuery.cpp,v 1.8 2010/09/24 21:30:52 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "Resolver_private.hh"
#include "FullTextQuery.hh"
#include "FullTextIndexer.hh"
#include "Indexing/IndexPrivate.hh"
#include "FerrisSlaveProcess.hh"


using namespace std;

namespace Ferris
{
    /**
     * Support for full text indexing and querys
     */
    namespace FullTextIndex 
    {
        typedef Loki::SingletonHolder< Loki::Factory< FullTextQuery, QueryMode > > QueryFactory;

        static bool shouldRunFullTextQueryFilesystemAsync_val = false;
        bool shouldRunFullTextQueryFilesystemAsync()
        {
            return shouldRunFullTextQueryFilesystemAsync_val;
        }
        
        void shouldRunFullTextQueryFilesystemAsync( bool v )
        {
            shouldRunFullTextQueryFilesystemAsync_val = v;
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        FullTextQuery::FullTextQuery( const std::string& s, fh_idx idx )
            :
            m_idx( idx ),
            m_accumulatorsMaxSize( 1000 ),
            m_resultSetMaxSize( 100 ),
            m_queryString( s ),
            m_asyncSelection( 0 ),
            m_asyncSlave( 0 ),
            m_limit( 0 )
        {
        }

        FullTextQuery::~FullTextQuery()
        {
        }
        
        
        void FullTextQuery::setAccumulatorsMaxSize( int v )
        {
            m_accumulatorsMaxSize = v;
        }
        
        void FullTextQuery::setResultSetMaxSize( int v )
        {
            m_resultSetMaxSize = v;
        }
        
        void FullTextQuery::setQuery( const std::string& s )
        {
            m_queryString = s;
        }

        void
        FullTextQuery::setLimit( int limit )
        {
            m_limit = limit;
        }
        
        

        void
        FullTextQuery::setIndex( fh_idx idx )
        {
            m_idx = idx;
        }
        
        void
        FullTextQuery::OnAsyncXMLMessage( fh_xstreamcol h )
        {
            stringmap_t& m = h->getStringMap();
            LG_IDX_D << "FullTextQuery::OnAsyncXMLMessage() m.sz:" << m.size() << endl;
            for( stringmap_t::iterator mi = m.begin(); mi != m.end(); ++mi )
            {
                LG_IDX_D << "FullTextQuery::OnAsyncXMLMessage() k:" << mi->first << " v:" << mi->second << endl;
            }
            
            if( m.end() != m.find("count") )
            {
                LG_IDX_D << "result count:" << m["count"] << endl;
            }
            else if( m.end() != m.find("docids") )
            {
                stringlist_t sl = Util::parseCommaSeperatedList( m["docids"] );
                for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    string docid = *si;
                    LG_IDX_D << "docid:" << docid << endl;
                }
            }
            else if( m.end() != m.find("earl") )
            {
                LG_IDX_D << "have earl!" << endl;
                string earl = m["earl"];
                LG_IDX_D << earl << endl;

                try
                {
                    string earl = m["earl"];
                    string docid = m["docid"];
                    
                    fh_context  ctx = Resolve( earl );
                    m_asyncSelection->createSubContext( "", ctx );
                    LG_IDX_D << "added earl:" << earl << endl;
                }
                catch( exception& e )
                {
                    LG_IDX_W << "ERROR:" << e.what() << endl;
                    cerr << "ERROR:" << e.what() << endl;
                }
            }
            else if( m.end() != m.find("outofband-error") )
                LG_IDX_W << "ERROR:" << m["outofband-error"] << endl;
            else if( m.end() != m.find("outofband-starting") )
                LG_IDX_D << "starting..." << endl;
            else
            {
                LG_IDX_W << "strange xml_msg_arrived()" << endl;
            }
            
        }

        
        void
        FullTextQuery::OnAsyncChildComeplete( ChildStreamServer* css, fh_runner r, int status, int estatus )
        {
            LG_IDX_D << "OnChildComeplete() estatus:" << estatus << endl;
            /*
             * Make sure that all async IO calls have been accepted.
             */
            Main::processAllPendingEvents();
        }
        

        fh_context
        FullTextQuery::executeAsync()
        {
            /*
             * We make a selection of the resulting objects and pass it
             * back as a filesystem
             */
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );


            string query_string = m_queryString;
            
            fh_stringstream qss;
            qss << "findexquery -P " << m_idx->getPath() << " --ferris-internal-async-message-slave  "
//                << " --limit " << limit << " "
                << query_string;
            LG_IDX_D << "cmd:" << tostr(qss) << endl;
            

            m_asyncSelection = selection;

            m_asyncSlave = CreateFerrisSlaveProcess( "fulltext query", tostr(qss) );
            m_asyncSlave->getMessageArrivedSig().connect( sigc::mem_fun( *this, &_Self::OnAsyncXMLMessage ) );
            m_asyncSlave->getChildCompleteSig().connect(  sigc::mem_fun( *this, &_Self::OnAsyncChildComeplete ) );
            m_asyncSlave->getRunner()->Run();
            LG_IDX_D << "have started slave process..." << endl;
            
//             if( m_idx->isCustomFerrisIndex() )
//                 m_idx->executeRankedQuery( selection,
//                                            m_queryString,
//                                            m_accumulatorsMaxSize,
//                                            m_resultSetMaxSize );
            
            return selection;
        }
        

        /********************************************************************************/
        /********************************************************************************/

        std::string makeFullTextQueryFilterString( const std::string& qs,
                                                   QueryMode qm,
                                                   fh_idx idx )
        {
            fh_stringstream ss;
            ss << "(ftq";
            if( qm == QUERYMODE_RANKED )
                ss << "r";
            else
                ss << "b";
            ss << "=" << qs;
            ss << ")";
            return tostr(ss);
        }
        


        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_DLLLOCAL RankedFullTextQuery
            :
            public FullTextQuery
        {
            
        public:

            RankedFullTextQuery( const std::string& s = "" , fh_idx idx = 0 )
                :
                FullTextQuery( s, idx )
                {
                }
            
            virtual fh_context execute();

            static bool reged;
        };
        bool RankedFullTextQuery::reged = QueryFactory::Instance().Register(
            QUERYMODE_RANKED, &MakeObject<FullTextQuery,RankedFullTextQuery>::Create );
        
        fh_context
        RankedFullTextQuery::execute()
        {
            /*
             * We make a selection of the resulting objects and pass it
             * back as a filesystem
             */
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );

            if( m_idx->isCustomFerrisIndex() )
                m_idx->executeRankedQuery( selection,
                                           m_queryString,
                                           m_accumulatorsMaxSize,
                                           m_resultSetMaxSize );
            
            return selection;
        }

        
        
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_DLLLOCAL BooleanFullTextQuery
            :
            public FullTextQuery
        {
            
        public:
            BooleanFullTextQuery( const std::string& s = "", fh_idx idx = 0 )
                :
                FullTextQuery( s, idx )
                {
                }
            
            virtual fh_context execute();

            static bool reged;
        };
        bool BooleanFullTextQuery::reged = QueryFactory::Instance().Register(
            QUERYMODE_BOOLEAN, &MakeObject<FullTextQuery,BooleanFullTextQuery>::Create );

        /**
         * Mount a boolean query string (eg. A & B | C ) as a filesystem
         */
        fh_context MountBooleanQueryString( const string& v )
        {
            RootContextFactory fac;

            fac.setContextClass( "fulltextboolean" );
            fac.AddInfo( RootContextFactory::ROOT, "/" );
            fac.AddInfo( "StaticString", v );

            fh_context c = fac.resolveContext( RESOLVE_EXACT );
            c->read();
            return c;
        }

        /**
         * assume the query is mounted at q and execute the part at q returning
         * result. This method may recurse to obtain subquery results.
         */
        docNumSet_t& ExecuteBooleanQuery( fh_context q,
                                          docNumSet_t& docnums,
                                          fh_idx idx,
                                          int limit )
        {
            string token   = getStrAttr( q, "token", "" );
            string tokenfc = foldcase(token);
            fh_attribute orderedtla = q->getAttribute("in-order-insert-list");
            fh_istream   orderedtls = orderedtla->getIStream();
            
            if( tokenfc == "&" )
            {
                string s;
                getline( orderedtls, s );
                fh_context l = q->getSubContext( s );
                getline( orderedtls, s );
                fh_context r = q->getSubContext( s );

                docNumSet_t ldocs;
                docNumSet_t rdocs;
                docNumSet_t tmp;

                ExecuteBooleanQuery( l, ldocs, idx, limit );
                ExecuteBooleanQuery( r, rdocs, idx, limit );
                set_intersection( ldocs.begin(), ldocs.end(),
                                  rdocs.begin(), rdocs.end(),
                                  inserter( tmp, tmp.begin() ) );
                docnums.insert( tmp.begin(), tmp.end() );
            }
            else if( tokenfc == "|" )
            {
                string s;
                getline( orderedtls, s );
                fh_context l = q->getSubContext( s );
                getline( orderedtls, s );
                fh_context r = q->getSubContext( s );

                docNumSet_t ldocs;
                docNumSet_t rdocs;
                docNumSet_t tmp;

                ExecuteBooleanQuery( l, ldocs, idx, limit );
                ExecuteBooleanQuery( r, rdocs, idx, limit );
                set_union( ldocs.begin(), ldocs.end(),
                           rdocs.begin(), rdocs.end(),
                           inserter( tmp, tmp.begin() ) );
                docnums.insert( tmp.begin(), tmp.end() );
            }
            else if( tokenfc == "-" )
            {
                string s;
                getline( orderedtls, s );
                fh_context l = q->getSubContext( s );
                getline( orderedtls, s );
                fh_context r = q->getSubContext( s );

                docNumSet_t ldocs;
                docNumSet_t rdocs;
                docNumSet_t tmp;

                ExecuteBooleanQuery( l, ldocs, idx, limit );
                ExecuteBooleanQuery( r, rdocs, idx, limit );
                set_difference( ldocs.begin(), ldocs.end(),
                                rdocs.begin(), rdocs.end(),
                                inserter( tmp, tmp.begin() ) );
                docnums.insert( tmp.begin(), tmp.end() );
            }
            else if( tokenfc == "!" )
            {
                cerr << "WARNING, negation is not coded yet!" << endl;
            }
            else
            {
                if( !idx->isCaseSensitive() )
                {
                    token = foldcase( token );
                }
                token = stem( token, idx->getStemMode() );

                idx->addAllDocumentsMatchingTerm( token, docnums, limit );
                
//                 fh_lexicon      lex  = idx->getLexicon();
//                 fh_invertedfile inv  = idx->getInvertedFile();

//                 LG_IDX_D << "looking up token:" << token << endl;
//                 termid_t tid = lex->lookup( token );
//                 if( !tid )
//                 {
//                     LG_IDX_D << "token not found in lexicon:" << token << endl;
//                     return docnums;
//                 }
            
//                 fh_term term = inv->getTerm( tid );

//                 docNumSet_t tmp;
//                 term->getDocumentNumbers( tmp );
//                 docnums.insert( tmp.begin(), tmp.end() );
            }
            return docnums;
        }

        
        fh_context
        BooleanFullTextQuery::execute()
        {
            string query_string = m_queryString;

            try
            {
                
                /*
                 * We make a selection of the resulting objects and pass it
                 * back as a filesystem
                 */
                fh_context selfactory = Resolve( "selectionfactory://" );
                fh_context selection  = selfactory->createSubContext( "" );

                if( m_queryString.empty() )
                {
                    return selection;
                }
                
                LG_IDX_D << "ExecuteBooleanQuery() query:" << query_string
                         << " limit:" << m_limit
                         << endl;

                /*
                 * Assume by default that boolean queries without any operators are
                 * using implicit & operators
                 */
                if( string::npos != query_string.find(' ')
                    && string::npos == query_string.find('&')
                    && string::npos == query_string.find('|')
                    && string::npos == query_string.find('-') )
                {
                    bool v = true;
                    fh_stringstream iss;
                    fh_stringstream oss;
                    iss << query_string;
                    string s;
                    while( getline( iss, s, ' ') )
                    {
                        if( !v )
                            oss << " & ";
                        oss << s;
                        v = false;
                    }
                    query_string = tostr(oss);
                }
            
                fh_context q = MountBooleanQueryString( query_string );
                fh_context root_of_q = q->getSubContext( *(q->getSubContextNames().begin()) );

                docNumSet_t docnums;
                ExecuteBooleanQuery( root_of_q, docnums, m_idx, m_limit );

                selection->addAttribute( "filter",
                                         makeFullTextQueryFilterString( m_queryString,
                                                                        QUERYMODE_BOOLEAN,
                                                                        m_idx ),
                                         FXD_FFILTER,
                                         true );

                stringlist_t NonResolvables;
                for( docNumSet_t::iterator iter = docnums.begin(); iter != docnums.end(); ++iter )
                {
                    LG_IDX_D << "  Found matching document:" << *iter << endl;

                    try
                    {
                        string earl = m_idx->resolveDocumentID( *iter );
                        try
                        {
                            fh_context ctx = Resolve( earl );
                            selection->createSubContext( "", ctx );
                        }
                        catch( exception& e )
                        {
                            NonResolvables.push_back( earl );
                            cerr << "Warning, e:" << e.what() << endl;
                        }
                    }
                    catch( exception& e )
                    {
                        cerr << "Warning, e:" << e.what() << endl;
                    }
                }
                m_idx->queryFoundNonResolvableURLs( NonResolvables );

                return selection;
            }
            catch( CanNotReadContextPcctsParseFailed& e )
            {
                stringstream ss;
                const stringlist_t& sl = e.getSyntaxErrorList();
                ss << "Syntax error for query:" << endl
                   << query_string << endl;
                for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    ss << *si << endl;
                }
                Throw_FulltextQuerySyntaxError( tostr(ss), 0 );
            }
            catch(...)
            {
                throw;
            }
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_DLLLOCAL XapianFullTextQuery
            :
            public FullTextQuery
        {
            
        public:

            XapianFullTextQuery( const std::string& s = "" , fh_idx idx = 0 )
                :
                FullTextQuery( s, idx )
                {
                }
            
            virtual fh_context execute();

            static bool reged;
        };
        bool XapianFullTextQuery::reged = QueryFactory::Instance().Register(
            QUERYMODE_XAPIAN, &MakeObject<FullTextQuery,XapianFullTextQuery>::Create );
        
        fh_context
        XapianFullTextQuery::execute()
        {
            /*
             * We make a selection of the resulting objects and pass it
             * back as a filesystem
             */
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );

            docNumSet_t docnums;
            m_idx->ExecuteXapianFullTextQuery( m_queryString, docnums, m_limit );
            
            stringlist_t NonResolvables;
            for( docNumSet_t::iterator iter = docnums.begin(); iter != docnums.end(); ++iter )
            {
                LG_IDX_D << "  Found matching document:" << *iter << endl;

                try
                {
                    string earl = m_idx->resolveDocumentID( *iter );
                    try
                    {
                        fh_context ctx = Resolve( earl );
                        selection->createSubContext( "", ctx );
                    }
                    catch( exception& e )
                    {
                        NonResolvables.push_back( earl );
                        cerr << "Warning, e:" << e.what() << endl;
                    }
                }
                catch( exception& e )
                {
                    cerr << "Warning, e:" << e.what() << endl;
                }
            }
            m_idx->queryFoundNonResolvableURLs( NonResolvables );

            return selection;
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_DLLLOCAL Tsearch2FullTextQuery
            :
            public FullTextQuery
        {
            
        public:

            Tsearch2FullTextQuery( const std::string& s = "" , fh_idx idx = 0 )
                :
                FullTextQuery( s, idx )
                {
                }
            
            virtual fh_context execute();

            static bool reged;
        };
        bool Tsearch2FullTextQuery::reged = QueryFactory::Instance().Register(
            QUERYMODE_TSEARCH2, &MakeObject<FullTextQuery,Tsearch2FullTextQuery>::Create );
        
        fh_context
        Tsearch2FullTextQuery::execute()
        {
            /*
             * We make a selection of the resulting objects and pass it
             * back as a filesystem
             */
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );

            docNumSet_t docnums;
            m_idx->ExecuteTsearch2FullTextQuery( m_queryString, docnums, m_limit );
            
            stringlist_t NonResolvables;
            for( docNumSet_t::iterator iter = docnums.begin(); iter != docnums.end(); ++iter )
            {
                LG_IDX_D << "  Found matching document:" << *iter << endl;

                try
                {
                    string earl = m_idx->resolveDocumentID( *iter );
                    try
                    {
                        fh_context ctx = Resolve( earl );
                        selection->createSubContext( "", ctx );
                    }
                    catch( exception& e )
                    {
                        NonResolvables.push_back( earl );
                        cerr << "Warning, e:" << e.what() << endl;
                    }
                }
                catch( exception& e )
                {
                    cerr << "Warning, e:" << e.what() << endl;
                }
            }
            m_idx->queryFoundNonResolvableURLs( NonResolvables );
            
            return selection;
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_DLLLOCAL BeagleFullTextQuery
            :
            public FullTextQuery
        {
            
        public:

            BeagleFullTextQuery( const std::string& s = "" , fh_idx idx = 0 )
                :
                FullTextQuery( s, idx )
                {
                }
            
            virtual fh_context execute();

            static bool reged;
        };
        bool BeagleFullTextQuery::reged = QueryFactory::Instance().Register(
            QUERYMODE_BEAGLE, &MakeObject<FullTextQuery,BeagleFullTextQuery>::Create );
        
        fh_context
        BeagleFullTextQuery::execute()
        {
            /*
             * We make a selection of the resulting objects and pass it
             * back as a filesystem
             */
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );

            docNumSet_t docnums;
            m_idx->ExecuteBeagleFullTextQuery( m_queryString, docnums, m_limit );
            
            stringlist_t NonResolvables;
            for( docNumSet_t::iterator iter = docnums.begin(); iter != docnums.end(); ++iter )
            {
                LG_IDX_D << "  Found matching document:" << *iter << endl;

                try
                {
                    string earl = m_idx->resolveDocumentID( *iter );
                    try
                    {
                        fh_context ctx = Resolve( earl );
                        selection->createSubContext( "", ctx );
                    }
                    catch( exception& e )
                    {
                        NonResolvables.push_back( earl );
                        cerr << "Warning, e:" << e.what() << endl;
                    }
                }
                catch( exception& e )
                {
                    cerr << "Warning, e:" << e.what() << endl;
                }
            }
            m_idx->queryFoundNonResolvableURLs( NonResolvables );

            return selection;
        }
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        class FERRISEXP_DLLLOCAL ExternalFullTextQuery
            :
            public FullTextQuery
        {
            
        public:

            ExternalFullTextQuery( const std::string& s = "" , fh_idx idx = 0 )
                :
                FullTextQuery( s, idx )
                {
                }
            
            virtual fh_context execute();

            static bool reged;
        };
        bool ExternalFullTextQuery::reged = QueryFactory::Instance().Register(
            QUERYMODE_EXTERNAL, &MakeObject<FullTextQuery,ExternalFullTextQuery>::Create );
        
        fh_context
        ExternalFullTextQuery::execute()
        {
            /*
             * We make a selection of the resulting objects and pass it
             * back as a filesystem
             */
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selection  = selfactory->createSubContext( "" );

            docNumSet_t docnums;
            m_idx->ExecuteExternalFullTextQuery( m_queryString, docnums, m_limit );
            
            stringlist_t NonResolvables;
            for( docNumSet_t::iterator iter = docnums.begin(); iter != docnums.end(); ++iter )
            {
                LG_IDX_D << "  Found matching document:" << *iter << endl;

                try
                {
                    string earl = m_idx->resolveDocumentID( *iter );
                    try
                    {
                        fh_context ctx = Resolve( earl );
                        selection->createSubContext( "", ctx );
                    }
                    catch( exception& e )
                    {
                        NonResolvables.push_back( earl );
                        cerr << "Warning, e:" << e.what() << endl;
                    }
                }
                catch( exception& e )
                {
                    cerr << "Warning, e:" << e.what() << endl;
                }
            }
            m_idx->queryFoundNonResolvableURLs( NonResolvables );
            
            return selection;
        }
        
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        namespace Factory 
        {
            fh_ftquery makeFullTextQuery( const std::string& s,
                                          QueryMode qm,
                                          fh_idx idx,
                                          int limit )
            {
                if( qm == QUERYMODE_USERPREF )
                    qm = QUERYMODE_RANKED;
                if( !isBound( idx ) )
                    idx = Factory::getDefaultFullTextIndex();
                
                fh_ftquery ret = QueryFactory::Instance().CreateObject( qm );

                ret->setIndex( idx );
                ret->setQuery( s );
                ret->setLimit( limit );
                
                return ret;
            }
        };


        fh_context ExecuteQuery( const std::string& s, QueryMode qm, fh_idx idx, int limit )
        {
            LG_IDX_D << "ExecuteQuery() query:" << s
                     << " mode:" << qm
                     << " limit:" << limit
                     << endl;
            if( idx )
                LG_IDX_D << " idx:" << idx->getPath() << endl;

            
            fh_ftquery q = Factory::makeFullTextQuery( s, qm, idx, limit );
            return q->execute();
        }

        fh_context ExecuteQueryAsync( const std::string& s, QueryMode qm, fh_idx idx, int limit )
        {
            LG_IDX_D << "ExecuteQueryAsync() query:" << s
                     << " mode:" << qm
                     << endl;
            if( idx )
                LG_IDX_D << " idx:" << idx->getPath() << endl;
            
            fh_ftquery q = Factory::makeFullTextQuery( s, qm, idx, limit );
            fh_context ret = q->executeAsync();
            ret->addHandlableToBeReleasedWithContext( GetImpl(q) );
            return ret;
        }
        
        
    };
};
