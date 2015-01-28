/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris-myrdf-inference
    Copyright (C) 2005 Ben Martin

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

    $Id: ferris-myrdf-inference.cpp,v 1.5 2010/09/24 21:31:19 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
 * 
*/


#include <Ferris.hh>
#include <FerrisBoost.hh>
#include <FerrisSemantic.hh>
#include <Regex.hh>
#include <popt.h>

#include <Configuration_private.hh>
#include <EAIndexer_private.hh>
#include <STLdb4/stldb4.hh>
using namespace ::STLdb4;

using namespace std;
using namespace Ferris;
using namespace Ferris::RDFCore;
using namespace Ferris::Semantic;

const string PROGRAM_NAME = "ferris-myrdf-inference";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

typedef list< pair< fh_node, fh_node > > ml_t;
unsigned long Verbose             = 0;
unsigned long Dump                = 0;
unsigned long DumpRAW             = 0;
unsigned long RematchUUIDNodes    = 0;
const char*   RematchUUIDNodes_ZeroSet_UnifyRegex_CSTR = "\\.(zip|rar|mkv|avi|mpg)$";
unsigned long RematchUUIDNodes_FoldManySet_ToSmushGroupLeader = 1;


fh_SmushSet tryToGetImplicitTreeSmush( string earl )
{
    if( !ends_with( earl, "/" ) )
        earl += "/";
    
    fh_TreeSmushing ts = getDefaultImplicitTreeSmushing();
    const TreeSmushing::m_smushSets_t& a = ts->getAll();
    TreeSmushing::m_smushSets_t::const_iterator ai = a.begin();
    TreeSmushing::m_smushSets_t::const_iterator ae = a.end();

        
    for( ; ai != ae ; ++ai )
    {
        fh_SmushSet ret = ai->second;
        const SmushSet::m_smushes_t& sm = ret->getSmushes();
        SmushSet::m_smushes_t::const_iterator si = sm.begin();
        SmushSet::m_smushes_t::const_iterator se = sm.end();

        for( ; si != se; ++si )
        {
            const string& n = si->first;
            fh_regex r = si->second;

            if( (*r)( earl ) )
            {
                return ret;
            }
        }
    }

    return 0;
}

void
FixUUIDLinksToPointToUUID( fh_model m, fh_node uuidnode, const ml_t& nodes )
{
    static fh_node uuidpred = Node::CreateURI( Semantic::getPredicateURI( "uuid" ) );
    static fh_node  olduuid = Node::CreateURI( Semantic::getPredicateURI( "old-uuid" ) );
    
    for( ml_t::const_iterator niter = nodes.begin();
         niter != nodes.end(); ++niter )
    {
        fh_node earlnode = niter->first;
        fh_node u = niter->second;

        if( u != uuidnode )
        {
            LG_RDF_I << "performing for uri:" << earlnode->toString() << endl;
            LG_RDF_I << "  remove:" << u->toString() << endl;
            LG_RDF_I << "     add:" << uuidnode->toString() << endl;

            m->set( earlnode, uuidpred, uuidnode );
            m->insert( uuidnode, olduuid, u );
        }
    }
}


fh_node tryToGetSopranoEASubjectNode( fh_model m, fh_node uuidnode )
{
    return m->getObject( uuidnode, uuidOutOfBandPredNode() );
}

fh_node ensureSopranoEASubjectNode( fh_model m, fh_node uuidnode )
{
    fh_node ret = tryToGetSopranoEASubjectNode( m, uuidnode );
    if( ret )
        return ret;
    
    fh_node mdnode = m->CreateBlankNode();
    m->set( uuidnode, uuidOutOfBandPredNode(), mdnode );
    setUUIDNodeModificationTime( uuidnode );
    return mdnode;
}




void
UUIDNode_RDFMerge( fh_model m, fh_node source, fh_node target )
{
    m->RDFMerge( source, target );

    //
    // Merge EA for source into target.
    //
    fh_node source_eanode = ensureSopranoEASubjectNode( m, source );
    fh_node target_eanode = ensureSopranoEASubjectNode( m, target );
    m->RDFMerge( source_eanode, target_eanode );

    
    setUUIDNodeModificationTime( target );
    Semantic::setSopranoEASubjectNode( target_eanode );
}


void
FixUUIDLinksMergeUUIDNodes( fh_model m, fh_node uuidnode, const ml_t& nodes )
{
    static fh_node uuidpred = Node::CreateURI( Semantic::getPredicateURI( "uuid" ) );
    static fh_node  olduuid = Node::CreateURI( Semantic::getPredicateURI( "old-uuid" ) );
    
    for( ml_t::const_iterator niter = nodes.begin();
         niter != nodes.end(); ++niter )
    {
        fh_node earlnode = niter->first;
        fh_node u = niter->second;

        if( u != uuidnode )
        {
            LG_RDF_I << "performing for uri:" << earlnode->toString() << endl;
            LG_RDF_I << "  merge all:" << u->toString() << endl;
            LG_RDF_I << "         to:" << uuidnode->toString() << endl;

            m->set( earlnode, uuidpred, uuidnode );
            m->insert( uuidnode, olduuid, u );
            UUIDNode_RDFMerge( m, u, uuidnode );
        }
    }
}


void
perhapsFixUUIDLinks( fh_model m, string fn, const ml_t& nodes )
{
    int count = nodes.size();
    static fh_node hn = Node::CreateURI( Semantic::getPredicateURI( "ferris-file-view-history" ) );
    
    LG_RDF_D << "fn:" << fn << " count:" << count << " ... ";

    Util::SingleShot virgin;
    bool combined = false;
    bool different = false;
    int numberSet = 0;
    fh_node un = 0;
    fh_node firstSet = 0;
    for( ml_t::const_iterator niter = nodes.begin();
         niter != nodes.end(); ++niter )
    {
        fh_node earlnode = niter->first;
        fh_node uuidnode = niter->second;
        bool v = m->getObject( uuidnode, hn );
        LG_RDF_D << " " << v;

        if( virgin() )
        {
            un = uuidnode;
            combined = v;
            numberSet += v;
            if( v )
                firstSet = uuidnode;
        }
        else
        {
            numberSet += v;
            if( v && !firstSet )
                firstSet = uuidnode;
            

            if( combined != v )
            {
                different = true;
            }

            if( un != uuidnode )
                different = true;
                
        }
    }
    LG_RDF_D << endl;

    if( different )
    {
        LG_RDF_D << "DIFFERENT! numberSet:" << numberSet << endl;
        if( firstSet )
        {
            LG_RDF_D << " firstSet:" << firstSet->toString();
        }
        LG_RDF_D << endl;
        LG_RDF_D << "------==============================-------------------------" << endl;
        for( ml_t::const_iterator niter = nodes.begin();
             niter != nodes.end(); ++niter )
        {
            fh_node earlnode = niter->first;
            fh_node uuidnode = niter->second;

            LG_RDF_D << "-------------------------------" << endl;
            LG_RDF_D << "  earl:" << earlnode->toString() << endl;
            LG_RDF_D << "  uuid:" << uuidnode->toString() << endl;
            LG_RDF_D << "  bound:" << (m->getObject( uuidnode, hn )) << endl;
                    
        }

        if( numberSet == 1 )
        {
            FixUUIDLinksToPointToUUID( m, firstSet, nodes );
        }
        else if( !numberSet )
        {
            static const fh_rex r = toregexh( RematchUUIDNodes_ZeroSet_UnifyRegex_CSTR );

            for( ml_t::const_iterator niter = nodes.begin();
                 niter != nodes.end(); ++niter )
            {
                fh_node earlnode = niter->first;
                fh_node uuidnode = niter->second;
            
                if( regex_search( earlnode->getURI()->toString(), r ) )
                {
                    FixUUIDLinksMergeUUIDNodes( m, un, nodes );
                    break;
                }
            }
        }
        else if( numberSet > 1 )
        {
            if( RematchUUIDNodes_FoldManySet_ToSmushGroupLeader )
            {
                for( ml_t::const_iterator niter = nodes.begin();
                     niter != nodes.end(); ++niter )
                {
                    fh_node earlnode = niter->first;
                    fh_node uuidnode = niter->second;

                    static const string smushLeaderRegexString = getEDBString(
                        FDB_GENERAL,
                        CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_K,
                        CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_DEFAULT );
                    static const fh_rex r = toregexh( smushLeaderRegexString );

                    if( regex_search( earlnode->getURI()->toString(), r ) )
                    {
                        FixUUIDLinksMergeUUIDNodes( m, uuidnode, nodes );
                        break;
                    }
                }
            }
        }
    }
}





int main( int argc, char** argv )
{
    int exit_status = 0;

    try
    {

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "show what is happening", "" },

                { "dump", 'd', POPT_ARG_NONE, &Dump, 0,
                  "dump out myrdf", "" },

                { "dump-raw", 'D', POPT_ARG_NONE, &DumpRAW, 0,
                  "dump out myrdf using more raw calls", "" },
                
                { "rematch-uuid-nodes", 'm', POPT_ARG_NONE, &RematchUUIDNodes, 0,
                  "if smushset regex have changed then rematch UUID nodes", "" },

                { "m-zero-set-unify-regex", 0, POPT_ARG_STRING, &RematchUUIDNodes_ZeroSet_UnifyRegex_CSTR, 0,
                  "regex to select which node to unify with when all uuidnodes are raw",
                  RematchUUIDNodes_ZeroSet_UnifyRegex_CSTR },



                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* queryfile");

        /* Now do options processing */
        {
            int c=-1;
            while ((c = poptGetNextOpt(optCon)) >= 0)
            {}
        }
        

        if (argc < 2)
        {
            poptPrintHelp(optCon, stderr, 0);
            exit(1);
        }

        fh_model m = getDefaultFerrisModel();
        fh_node uuidpred = Node::CreateURI( Semantic::getPredicateURI( "uuid" ) );
        fh_node hn = Node::CreateURI( Semantic::getPredicateURI( "ferris-file-view-history" ) );

        if( Dump )
        {
            m->write( Factory::fcerr() );
        }
        if( DumpRAW )
        {
            StatementIterator siter = m->begin();
            StatementIterator e     = m->end();

            cerr << "m.size:" << m->size() << endl;
            cerr << "s.size:" << distance( m->begin(), e ) << endl;

            for( ; siter != e; ++siter )
            {
                fh_statement st = *siter;
                fh_node s = st->getSubject();
                fh_node p = st->getPredicate();
                fh_node o = st->getObject();

                cerr << "Subject:" << s->toString()
                     << " Pred:" << p->toString()
                     << " Obj:" << o->toString()
                     << endl;
            }
        }

        if( RematchUUIDNodes )
        {
            StatementIterator siter = m->begin();
            StatementIterator e     = m->end();

//             cout << "m.size:" << m->size() << endl;
//             cout << "s.size:" << distance( m->begin(), e ) << endl;


            //
            //
            // smushset -> map( subject, object ) where predicate is uuid
            //
            typedef map< fh_node, fh_node > somap_t;
//            somap_t somap;
            typedef map< fh_SmushSet, somap_t > sm_t;
            sm_t sm;
                
            for( ; siter != e; ++siter )
            {
                fh_statement st = *siter;
                fh_node s = st->getSubject();
                fh_node p = st->getPredicate();
                fh_node o = st->getObject();

//                 cerr << "Subject:" << s->toString()
//                      << " Pred:" << p->toString()
//                      << " Obj:" << o->toString()
//                      << endl;

                if( p == uuidpred )
                {
                    string fullearl = s->getURI()->toString();
                    string parentearl = fullearl.substr( 0, fullearl.rfind("/") );
                    fh_SmushSet smushSet = tryToGetImplicitTreeSmush( parentearl );

                    if( smushSet )
                    {
                        sm[ smushSet ][ s ] = o;
                    }
                }
            }

            cout << "sm.sz:" << sm.size() << endl;
            for( sm_t::iterator smiter = sm.begin(); smiter != sm.end(); ++smiter )
            {
                fh_SmushSet ss = smiter->first;
                const somap_t& somap = smiter->second;

                if( ss )
                {
                    cout << "smush:" << ss->getName() << endl;
                    cout << "somap.sz:" << somap.size() << endl;

                    //
                    // For each filename, the list of matching (subj,obj) pairs
                    //
                    typedef map< string, ml_t > matches_t;
                    matches_t matches;

                    for( somap_t::const_iterator somapiter = somap.begin();
                         somapiter != somap.end(); ++somapiter )
                    {
                        fh_node subj = somapiter->first;
                        fh_node obj  = somapiter->second;

                        string fullearl = subj->getURI()->toString();
                        string fn = fullearl.substr( fullearl.rfind("/")+1 );

                        matches[ fn ].push_back( make_pair( subj, obj ) );
                    }

                    cout << "matches.sz:" << matches.size() << endl;

                    matches_t::iterator mend = matches.end();
                    for( matches_t::iterator miter = matches.begin(); miter != mend; ++miter )
                    {
                        const string& fn = miter->first;
                        const ml_t& nodes = miter->second;
                        int count = nodes.size();

                        if( count > 1 )
                        {
                            perhapsFixUUIDLinks( m, fn, nodes );
                        }
                    }
                }
            }
            
        }

        m->sync();
    }
    catch( exception& e )
    {
        cerr << "error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


