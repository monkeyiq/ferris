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

    $Id: EAQuery.cpp,v 1.23 2011/05/06 21:30:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/EAQuery.hh>
#include <Ferris/FilteredContext.hh>
#include <Ferris/FilteredContext_private.hh>
#include <Ferris/Medallion.hh>
#include <Ferris/ForwardEAIndexInterface.hh>
#include <Ferris/ChildStreamServer.hh>
#include <Ferris/FerrisSlaveProcess.hh>

#include <boost/regex.hpp>

#include <numeric>
#ifndef STLPORT
#include <ext/numeric>
#endif

using namespace std;


namespace Ferris
{
    namespace EAIndex 
    {
        #define RESULTSETMAXSIZE_DEFAULT 100

        static bool shouldRunEAQueryFilesystemAsync_val = false;
        
        bool shouldRunEAQueryFilesystemAsync()
        {
            return shouldRunEAQueryFilesystemAsync_val;
        }
        
        void shouldRunEAQueryFilesystemAsync( bool v )
        {
            shouldRunEAQueryFilesystemAsync_val = v;
        }
        
        EAQuery::EAQuery( const std::string& s, fh_idx idx )
            :
            m_idx( idx ),
            m_queryString( s ),
            m_resultSetMaxSize( RESULTSETMAXSIZE_DEFAULT )
        {
        }
            
        EAQuery::~EAQuery()
        {
        }
            

        void
        EAQuery::setIndex( fh_idx idx )
        {
            m_idx = idx;
        }
            
        void
        EAQuery::setQuery( const std::string& s )
        {
            m_queryString = s;
        }
            
        void
        EAQuery::setResultSetMaxSize( int v )
        {
            m_resultSetMaxSize = v;
        }

        void
        EAQuery::getQuerySQL( std::stringstream& SQLHeaderSS,
                              std::stringstream& SQLWherePredicatesSS,
                              std::stringstream& SQLTailerSS,
                              stringset_t& lookupTablesUsed,
                              bool& queryHasTimeRestriction,
                              string& DocIDColumn,
                              stringset_t& eanamesUsed,
                              MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo )
        {
            // cerr << "error... EAQuery::getQuerySQL() called" << endl;
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        class FERRISEXP_DLLLOCAL EAQuery_Heur
            :
            public EAQuery
        {
            typedef EAQuery_Heur _Self;

            fh_FerrisSlaveProcess m_asyncSlave;
            
//             fh_runner     m_asyncRunner;
//             fh_xstreamcol m_asyncXMLCol;
//             fh_childserv  m_asyncChildServ;
            
            fh_context    m_asyncSelection;
            fh_fwdeaidx   m_asyncFWD;
            
            void OnAsyncXMLMessage( fh_xstreamcol h );
            void OnAsyncChildComeplete( ChildStreamServer* css, fh_runner r, int status, int estatus );
            
        public:

            EAQuery_Heur( const std::string& s, fh_idx idx )
                :
                EAQuery( s, idx ),
                m_asyncSlave(0),
//                 m_asyncRunner(0),
//                 m_asyncXMLCol(0),
//                 m_asyncChildServ(0),
                m_asyncSelection(0),
                m_asyncFWD(0)
                {
                }
            
            virtual ~EAQuery_Heur()
                {
                    LG_EAIDX_D << "~EAQuery_Heur()" << endl;
                }

            docNumSet_t& ExecuteQueryToDocIDs( docNumSet_t& ret );
            docNumSet_t& ExecuteQueryToDocIDs( docNumSet_t& ret, int limit );

            fh_context ResolveDocIDs( docNumSet_t& s );
            fh_context ResolveDocIDs( docNumSet_t& s, fh_context selectionCtx );
            stringset_t& ResolveDocIDsToURLs( docNumSet_t& s, stringset_t& ret );

            virtual fh_context execute();
            virtual fh_context execute( fh_context selectionCtx, int limit = 0 );

            virtual fh_context executeAsync( fh_context selc, int limit = 0 );

            
            virtual void
            getQuerySQL( std::stringstream& SQLHeaderSS,
                         std::stringstream& SQLWherePredicatesSS,
                         std::stringstream& SQLTailerSS,
                         stringset_t& lookupTablesUsed,
                         bool& queryHasTimeRestriction,
                         string& DocIDColumn,
                         stringset_t& eanamesUsed,
                         MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo );
        };

        double getRange( string s )
        {
            int start = s.rfind("=")+1;
            int end = start+1  + s.substr( start+1 ).find("(");

            string r = s.substr( start, end );
            // cerr << " r:" << r << endl;
            return toType<double>( r );
        }
        
        docNumSet_t&
        EAQuery_Heur::ExecuteQueryToDocIDs( docNumSet_t& ret, int limit )
        {
            string query_string = m_queryString;

            if( m_queryString.empty() )
            {
                return ret;
            }
            LG_EAIDX_D << "EAQuery_Heur::execute(top) query:" << query_string << " limit:" << limit << endl;

            //
            // Where an emblem name is non unique, rewrite query to include
            // all emblems of the same name by id.
            //
            while( query_string.find( "(emblem:has-" ) != string::npos )
            {
                boost::regex subqregex("(.*)\\(emblem:has-([^<=]+)([<=]+)([0-9.]+)\\)(.*)");
                boost::smatch matches;
                if(boost::regex_match(query_string, matches, subqregex ))
                {
                    if( matches.size() == 6 )
                    {
                        stringstream qss;
                        string emblemName = matches[2];
                        string opcode = matches[3];
                        string v = matches[4];
                        
                        fh_etagere et = ::Ferris::Factory::getEtagere();
                        fh_emblem em = 0;

                        emblemset_t el;
                        el = et->getAllEmblemsWithName( el, emblemName );

                        if( el.empty() )
                        {
                            stringstream ss;
                            ss << "Query for emblem that does not exist:" << emblemName;
                            Throw_EmblemNotFoundException( tostr(ss), 0 );
                        }
                        
                        qss << emblemListToEAQuery( el, '&' );
                        
                        stringstream newq;
                        newq << matches[1];
                        newq << tostr(qss);
                        newq << matches[matches.size()-1];
                        LG_EAIDX_D << "new query:" << tostr(newq) << endl;

                        query_string = tostr(newq);
                    }
                }
            }
            
            while( query_string.find( "(ferris-near" ) != string::npos )
            {
                boost::regex subqregex("(.*)\\(ferris-near-([^<=]+)([<=]+)([0-9.]+)(km|miles|mile|d)?\\)(.*)");
                boost::smatch matches;
                if(boost::regex_match(query_string, matches, subqregex ))
                {
                    LG_EAIDX_D << "m.sz:" << matches.size() << endl;
                    LG_EAIDX_D << "prefix:" << matches.prefix() << endl;
                    LG_EAIDX_D << "suffix:" << matches.suffix() << endl;

                    for( int i=0; i < matches.size(); ++i )
                        LG_EAIDX_D << "match i:" << i << " is:" << matches[i] << endl;

                    if( matches.size() == 7 )
                    {
                        stringstream qss;

                        string emblemName = matches[2];
                        double range = toType<double>(matches[4]);
                        string units = matches[5];
                        bool ShowDownSet = false;
                        if( units.empty() )
                            units = "km";
                        
                        LG_EAIDX_D << "emblemName:" << emblemName
                             << " Range:" << range
                             << " units:" << units
                             << endl;

                        fh_etagere et = ::Ferris::Factory::getEtagere();
                        fh_emblem em = 0;

                        if( starts_with( emblemName, "id-" ))
                        {
                            string EmblemID = emblemName.substr(3);
                            em = et->getEmblemByID( toType<emblemID_t>( EmblemID ));
                        }
                        else
                        {
                            em = et->getEmblemByName( emblemName );
                        }

                        if( units == "km" )
                        {
                        }
                        else if( units == "miles" || units == "mile" )
                        {
                            range *= 1.609344;
                        }
                        if( units == "d" )
                        {
                            range = DRangeToKiloMeters( range );
                        }

                        emblemset_t result;
                        getEmblemsNear( result, em, range, et, ShowDownSet );
                        
//                         double elat = em->getDigitalLatitude();
//                         double elong = em->getDigitalLongitude();


//                         emblemset_t result;
//                         fh_emblem geospatialem = et->getEmblemByName( "libferris-geospatial" );
//                         emblems_t all = geospatialem->getDownset();
//                         for( emblems_t::iterator ai = all.begin(); ai != all.end(); ++ai )
//                         {
//                             double alat = (*ai)->getDigitalLatitude();
//                             double along = (*ai)->getDigitalLongitude();
//                             if( !alat || !along )
//                                 continue;
//                             if( fabs( alat - elat ) < range )
//                             {
//                                 if( fabs( along - elong ) < range )
//                                 {
//                                     if( ShowDownSet )
//                                     {
//                                         emblems_t ds = (*ai)->getDownset();
//                                         copy( ds.begin(), ds.end(), inserter( result, result.end() ) );
//                                     }
//                                     else
//                                     {
//                                         result.insert( *ai );
//                                     }
//                                 }
//                             }
//                         }

                        qss << emblemListToEAQuery( result );
//                         if( result.size() > 1 )
//                             qss << "(|";
//                         bool v = true;
//                         for( emblemset_t::iterator ei = result.begin(); ei!=result.end(); ++ei )
//                         {
//                             fh_emblem em = *ei;
//                             if( v ) v = false;
//                             else    qss << "";
        
//                             qss << "(emblem:id-" << em->getID() << "==1)";
//                         }
//                         if( result.size() > 1 )
//                             qss << ")";
                            
                        stringstream newq;
                        newq << matches[1];
                        newq << tostr(qss);
                        newq << matches[matches.size()-1];
                        LG_EAIDX_D << "new query:" << tostr(newq) << endl;

                        query_string = tostr(newq);
                    }
                }
            }
            
            
            LG_EAIDX_D << "EAQuery_Heur::execute() query:" << query_string << endl;
            fh_context q = Ferris::Factory::MakeFilter( query_string );
            if( q->getSubContextNames().empty() )
            {
                fh_stringstream ss;
                ss << "Invalid query given:" << query_string << endl
                   << "Can not find toplevel operation of query"
                   << endl;
                Throw_EAIndexException( tostr(ss), 0 );
            }
            fh_context root_of_q = q->getSubContext( *(q->getSubContextNames().begin()) );

            m_idx->ExecuteQuery( root_of_q, ret, this, limit );

            return ret;
        }
        
        
        docNumSet_t&
        EAQuery_Heur::ExecuteQueryToDocIDs( docNumSet_t& ret )
        {
            return ExecuteQueryToDocIDs( ret, 0 );
        }

        
        std::string
        EAQuery::resolveDocumentIDAttemptFromCache( docid_t docid, std::map< docid_t, std::string >& cache )
        {
            std::map< docid_t, std::string >::iterator ci = cache.find( docid );
            if( ci != cache.end() )
                return ci->second;
            if( !m_idx )
            {
                cerr << "no m_idx object!" << endl;
                return "";
            }
            
            return m_idx->resolveDocumentID( docid );
        }
        
        fh_context
        EAQuery_Heur::ResolveDocIDs( docNumSet_t& docnums, fh_context selection )
        {
            selection->addAttribute( "filter", m_queryString, FXD_FFILTER, true );
            selection->addAttribute( "query", m_queryString,  FXD_FFILTER, true );

            fh_fwdeaidx fwd = m_idx->tryToCreateForwardEAIndexInterface();
            LG_EAIDX_D << "EAQuery_Heur::ResolveDocIDs() fwd:" << toVoid(fwd) << endl;
            LG_EAIDX_D << "EAQuery_Heur::ResolveDocIDs() dn.sz:" << docnums.size() << endl;

            std::map< docid_t, std::string > cache;
            m_idx->precacheDocIDs( docnums, cache );
            
            stringlist_t NonResolvables;
            for( docNumSet_t::iterator iter = docnums.begin(); iter != docnums.end(); ++iter )
            {
                std::string earl = resolveDocumentIDAttemptFromCache( *iter, cache );
                try
                {
                    fh_context  ctx  = Resolve( earl );
                    selection->createSubContext( "", ctx );

                    if( fwd ) fwd->addDocumentID( *iter );
                }
                catch( exception& e )
                {
                    NonResolvables.push_back( earl );
                    cerr << "Warning, e:" << e.what() << endl;
                }
            }

            if( fwd )
            {
                if( SelectionContext* selc = dynamic_cast<SelectionContext*>( GetImpl( selection ) ))
                {
                    selc->setForwardEAIndexInterface( fwd );
                }
            }

            // 1.3.1+: these are no longer removed during the query itself...
            // you need to run compact() now.
//            m_idx->queryFoundNonResolvableURLs( NonResolvables );
            
//            cerr << "EAQuery.selection:" << selection->getURL() << endl;
            
            return selection;
        }
        
        
        
        fh_context
        EAQuery_Heur::ResolveDocIDs( docNumSet_t& docnums )
        {
            /*
             * We make a selection of the resulting objects and pass it
             * back as a filesystem
             */
            fh_context selfactory = Resolve( "selectionfactory://" );
//            cerr << "have selfactory" << endl;
            fh_context selection  = selfactory->createSubContext( "" );
//            cerr << "have sel" << endl;

            return ResolveDocIDs( docnums, selection );
        }
        
        stringset_t&
        EAQuery_Heur::ResolveDocIDsToURLs( docNumSet_t& docnums, stringset_t& ret )
        {
//            cerr << "EAQuery_Heur::ResolveDocIDsToURLs()" << endl;

            std::map< docid_t, std::string > cache;
            m_idx->precacheDocIDs( docnums, cache );
            
            stringlist_t NonResolvables;
            for( docNumSet_t::iterator iter = docnums.begin(); iter != docnums.end(); ++iter )
            {
                std::string earl = resolveDocumentIDAttemptFromCache( *iter, cache );
//                cerr << "earl:" << earl << endl;
                ret.insert( earl );
            }
            return ret;
        }
        

        fh_context
        EAQuery_Heur::execute( fh_context selection, int limit )
        {
            docNumSet_t docset;
            return ResolveDocIDs( ExecuteQueryToDocIDs( docset, limit ), selection );
        }
        
        
        fh_context
        EAQuery_Heur::execute()
        {
            docNumSet_t docset;
            return ResolveDocIDs( ExecuteQueryToDocIDs( docset ));
        }

        void
        EAQuery_Heur::OnAsyncXMLMessage( fh_xstreamcol h )
        {
            
            stringmap_t&   m = h->getStringMap();
            LG_EAIDX_D << "EAQuery_Heur::OnAsyncXMLMessage() m.sz:" << m.size() << endl;
            for( stringmap_t::iterator mi = m.begin(); mi != m.end(); ++mi )
            {
                LG_EAIDX_D << "EAQuery_Heur::OnAsyncXMLMessage() k:" << mi->first << " v:" << mi->second << endl;
            }
            
            if( m.end() != m.find("count") )
            {
                LG_EAIDX_D << "result count:" << m["count"] << endl;
            }
            else if( m.end() != m.find("docids") )
            {
                stringlist_t sl = Util::parseCommaSeperatedList( m["docids"] );
                for( stringlist_t::iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    string docid = *si;
                    
                    LG_EAIDX_D << "docid:" << docid << endl;
                    if( m_asyncFWD )
                        m_asyncFWD->addDocumentID( toint(docid) );
                }
            }
            else if( m.end() != m.find("earl") )
            {
                LG_EAIDX_D << "have earl!" << endl;
                string earl = m["earl"];
                LG_EAIDX_D << earl << endl;
                

                try
                {
                    string earl = m["earl"];
                    string docid = m["docid"];
                    
                    fh_context  ctx = Resolve( earl );
                    m_asyncSelection->createSubContext( "", ctx );
                    LG_EAIDX_D << "added earl:" << earl << endl;
                }
                catch( exception& e )
                {
                    LG_EAIDX_W << "ERROR:" << e.what() << endl;
                    cerr << "ERROR:" << e.what() << endl;
                }
            }
            else if( m.end() != m.find("outofband-error") )
                LG_EAIDX_W << "ERROR:" << m["outofband-error"] << endl;
            else if( m.end() != m.find("outofband-starting") )
                LG_EAIDX_D << "starting..." << endl;
            else
            {
                LG_EAIDX_W << "strange xml_msg_arrived()" << endl;
            }
            
        }

        void
        EAQuery_Heur::OnAsyncChildComeplete( ChildStreamServer* css, fh_runner r, int status, int estatus )
        {
            LG_EAIDX_D << "OnChildComeplete() estatus:" << estatus << endl;
            /*
             * Make sure that all async IO calls have been accepted.
             */
            Main::processAllPendingEvents();
//             m_asyncChildServ = 0;
//             m_asyncXMLCol = 0;
//             m_asyncRunner = 0;
//             m_asyncSelection = 0;
        }
        
        
        fh_context
        EAQuery_Heur::executeAsync( fh_context selection, int limit )
        {
//            docNumSet_t docset;
//            return ResolveDocIDs( ExecuteQueryToDocIDs( docset ));

            string query_string = m_queryString;

            
            
            fh_stringstream qss;
            qss << "feaindexquery -P " << m_idx->getPath() << " --ferris-internal-async-message-slave  "
                << " --limit " << limit << " "
                << query_string;
            LG_EAIDX_D << "cmd:" << tostr(qss) << endl;

            m_asyncSelection = selection;
            m_asyncFWD = m_idx->tryToCreateForwardEAIndexInterface();
            if( m_asyncFWD )
            {
                if( SelectionContext* selc = dynamic_cast<SelectionContext*>( GetImpl( selection ) ))
                {
                    selc->setForwardEAIndexInterface( m_asyncFWD );
                }
            }

            
            m_asyncSlave = CreateFerrisSlaveProcess( "ea query", tostr(qss) );
            m_asyncSlave->getMessageArrivedSig().connect( sigc::mem_fun( *this, &_Self::OnAsyncXMLMessage ) );
            m_asyncSlave->getChildCompleteSig().connect(  sigc::mem_fun( *this, &_Self::OnAsyncChildComeplete ) );
            fh_runner r = m_asyncSlave->getRunner();
            // This is needed or we might miss the last bit of output
            r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags() & ~(G_SPAWN_DO_NOT_REAP_CHILD)));
            r->Run();


            
//             m_asyncRunner = new Runner();
//             m_asyncRunner->setCommandLine( tostr(qss) );
//             m_asyncRunner->setSpawnFlags(
//                 GSpawnFlags(
//                     G_SPAWN_SEARCH_PATH |
//                     G_SPAWN_STDERR_TO_DEV_NULL |
//                     G_SPAWN_DO_NOT_REAP_CHILD ));

//             m_asyncXMLCol = ::Ferris::Factory::MakeXMLStreamCol();
//             m_asyncXMLCol->getMessageArrivedSig().connect( sigc::mem_fun( *this, &_Self::OnAsyncXMLMessage ) );
//             m_asyncXMLCol->attach(m_asyncRunner);

//             m_asyncChildServ = new ChildStreamServer();
//             m_asyncChildServ->addChild(m_asyncRunner);
//             m_asyncChildServ->getChildCompleteSig().connect( sigc::mem_fun( *this, &_Self::OnAsyncChildComeplete ) );

//             m_asyncRunner->Run();

//             fh_context selfactory = Resolve( "selectionfactory://" );
//             fh_context selection  = selfactory->createSubContext( "" );

            LG_EAIDX_D << "EAQuery_Heur::executeAsync() returning" << endl;
//            Main::mainLoop();

            return m_asyncSelection;
        }
        
        void
        EAQuery_Heur::getQuerySQL( std::stringstream& SQLHeaderSS,
                                   std::stringstream& SQLWherePredicatesSS,
                                   std::stringstream& SQLTailerSS,
                                   stringset_t& lookupTablesUsed,
                                   bool& queryHasTimeRestriction,
                                   string& DocIDColumn,
                                   stringset_t& eanamesUsed,
                                   MetaEAIndexerInterface::BuildQuerySQLTermInfo_t& termInfo )
        {
            LG_EAIDX_D << "EAQuery_Heur::getQuerySQL()" << endl;
            
            docNumSet_t docset;
            string query_string = m_queryString;

            if( m_queryString.empty() )
            {
                return;
            }
            LG_EAIDX_D << "EAQuery_Heur::getQuerySQL() query:" << query_string << endl;

            fh_context q = Ferris::Factory::MakeFilter( query_string );
            if( q->getSubContextNames().empty() )
            {
                fh_stringstream ss;
                ss << "Invalid query given:" << query_string << endl
                   << "Can not find toplevel operation of query"
                   << endl;
                Throw_EAIndexException( tostr(ss), 0 );
            }
            fh_context root_of_q = q->getSubContext( *(q->getSubContextNames().begin()) );

            m_idx->BuildQuerySQL( root_of_q, docset, this,
                                  SQLHeaderSS,
                                  SQLWherePredicatesSS,
                                  SQLTailerSS,
                                  lookupTablesUsed,
                                  queryHasTimeRestriction,
                                  DocIDColumn,
                                  eanamesUsed,
                                  termInfo );

            return;
        }
        
        

        
        
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        fh_context ExecuteQuery( const std::string& qs, fh_idx idx )
        {
            fh_eaquery q = Factory::makeEAQuery( qs, idx );
            return q->execute();
        }

        fh_context ExecuteQueryAsync( const std::string& qs, fh_idx idx )
        {
            return ExecuteQueryAsync( qs, 0, idx );
        }

        fh_context ExecuteQueryAsync( const std::string& qs, int limit, fh_idx idx )
        {
            return ExecuteQueryAsync( qs, idx, 0, limit );
        }
        fh_context ExecuteQueryAsync( const std::string& qs, fh_idx idx,
                                      fh_context selectionCtx,
                                      int limit )
        {
            fh_eaquery q = Factory::makeEAQuery( qs, idx );
//            // FIXME: leak!
//            q->AddRef();
            LG_EAIDX_D << "ExecuteQueryAsync() executing query:" << qs << endl;
            if( !isBound( selectionCtx ) )
            {
                fh_context selfactory = Resolve( "selectionfactory://" );
                selectionCtx  = selfactory->createSubContext( "" );
            }
            fh_context ret = q->executeAsync( selectionCtx, limit );
            ret->addHandlableToBeReleasedWithContext( GetImpl(q) );
            return ret;
        }
        
        fh_context ExecuteQuery( const std::string& qs, int limit )
        {
            fh_eaquery q = Factory::makeEAQuery( qs, 0 );
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selectionCtx  = selfactory->createSubContext( "" );
            return q->execute( selectionCtx, limit );
        }
        
        fh_context ExecuteQuery( const std::string& qs, int limit, long& totalNumberOfMatches )
        {
            totalNumberOfMatches = 0;
    
            fh_eaquery q = Factory::makeEAQuery( qs, 0 );
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selectionCtx  = selfactory->createSubContext( "" );
            q->execute( selectionCtx, limit );

            docNumSet_t docset;
            q->ExecuteQueryToDocIDs( docset );
            totalNumberOfMatches = docset.size();
            if( totalNumberOfMatches > limit )
            {
                docNumSet_t::iterator iter = docset.begin();
                advance( iter, limit );
                docset.erase( iter, docset.end() );
            }
            fh_context c = q->ResolveDocIDs( docset );
            return c;
        }

        fh_context ExecuteQuery( const std::string& qs, int limit, bool& morePossible )
        {
            morePossible = false;
            limit++;
            
            fh_eaquery q = Factory::makeEAQuery( qs, 0 );
            fh_context selfactory = Resolve( "selectionfactory://" );
            fh_context selectionCtx  = selfactory->createSubContext( "" );
            q->execute( selectionCtx, limit );

            docNumSet_t docset;
            q->ExecuteQueryToDocIDs( docset );
            if( docset.size() >= limit )
            {
                morePossible = true;

                docNumSet_t::iterator iter = docset.begin();
                advance( iter, limit );
                docset.erase( iter, docset.end() );
            }
            fh_context c = q->ResolveDocIDs( docset );
            return c;
        }

        fh_context ExecuteQuery( const std::string& qs, fh_idx idx,
                                 fh_context selectionCtx, int limit )
        {
            fh_eaquery q = Factory::makeEAQuery( qs, idx );
            return q->execute( selectionCtx, limit );
        }

        docNumSet_t& ExecuteQueryToDocIDs( const std::string& qs,
                                           fh_idx idx,
                                           docNumSet_t& ret,
                                           int limit )
        {
            fh_eaquery q = Factory::makeEAQuery( qs, idx );
            q->ExecuteQueryToDocIDs( ret, limit );
            return ret;
        }
        
        stringset_t& ExecuteQueryToToURLs( const std::string& qs,
                                           fh_idx idx,
                                           stringset_t& ret,
                                           int limit )
        {
            docNumSet_t docs;
            fh_eaquery q = Factory::makeEAQuery( qs, idx );
            q->ExecuteQueryToDocIDs( docs, limit );
            q->ResolveDocIDsToURLs( docs, ret );
            return ret;
        }
    

        
        namespace Factory 
        {
            /**
             * Used for more advanced queries than ExecuteQuery() can provide.
             * first call here to create a Query object and then you can setup
             * the details of the query and other tweaks that are set to default
             * values if ExecuteQuery() is used.
             */
            fh_eaquery makeEAQuery( const std::string& qs, fh_idx idx )
            {
                if( !isBound( idx ) )
                    idx = Factory::getDefaultEAIndex();
                
                fh_eaquery ret = new EAQuery_Heur( qs, idx );

                ret->setIndex( idx );
                ret->setQuery( qs );

                return ret;
            }
        };


        
    };
};


