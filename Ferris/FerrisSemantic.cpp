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

    $Id: FerrisSemantic.cpp,v 1.4 2011/04/08 21:30:17 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <climits>

#include <errno.h>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/set.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/split_member.hpp>


#include <Ferris/Ferris.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/FerrisBoost.hh>
#include <Ferris/Regex.hh>
#include <Ferris/FerrisSemantic.hh>

using namespace std;

//#undef  LG_RDF_D
//#define LG_RDF_D cerr

namespace Ferris
{
    namespace Semantic
    {
        namespace Wordnet
        {
            
            // word offset after the # in a word node [L#102037721cat]
            static const int WORD_NODE_TEXT_OFFSET = 9;

            string RDFSCHEMA = "http://www.dcc.uchile.cl/~agraves/wordnet/1.0/schema#";
            string WNPFX = "http://www.dcc.uchile.cl/~agraves/wordnet/1.0/";

            fh_node& hasWord()
            {
                static fh_node ret = Node::CreateURI( RDFSCHEMA + "hasWord" );
                return ret;
            }
            fh_node& hasSynSet()
            {
                static fh_node ret = Node::CreateURI( RDFSCHEMA + "hasSynSet" );
                return ret;
            }
            fh_node& NounSynSet()
            {
                static fh_node ret = Node::CreateURI( RDFSCHEMA + "NounSynSet" );
                return ret;
            }
            fh_node& Type()
            {
                static fh_node ret = Node::CreateURI( "http://www.w3.org/1999/02/22-rdf-syntax-ns#type" );
                return ret;
            }
            fh_node& hasGloss()
            {
                static fh_node ret = Node::CreateURI( RDFSCHEMA + "hasGloss" );
                return ret;
            }
            fh_node& hyponymOf()
            {
                static fh_node ret = Node::CreateURI( RDFSCHEMA + "hyponymOf" );
                return ret;
            }
            fh_node& partMeronymOf()
            {
                static fh_node ret = Node::CreateURI( RDFSCHEMA + "partMeronymOf" );
                return ret;
            }
            
            ::Ferris::RDFCore::nodelist_t& getSynSet( fh_model m,
                                                      ::Ferris::RDFCore::nodelist_t& ret,
                                                      const std::string& word )
            {
                fh_node wordNode = Node::CreateURI( WNPFX + "words#" + word );
                
                NodeIterator end   = NodeIterator();
                NodeIterator oiter = m->findSubjects( hasWord(), wordNode );
                for( ; oiter != end; ++oiter )
                {
                    fh_node subject = *oiter;
                    NodeIterator oiter = m->findObjects( subject, hasSynSet() );
                    for( ; oiter != end; ++oiter )
                    {
                        fh_node synset = *oiter;
                        ret.push_back( synset );
                    }
                }
                return ret;
            }
            
            std::string getNodeID( fh_node n )
            {
                stringstream ret;
                string s = n->getURI()->toString();
                ret << " id=\"" << s.substr( s.rfind("#")+1 ) << "\" ";
                return ret.str();
            }
                
            std::string getWordNodeAsString( fh_node n )
            {
                stringstream ret;
                string s = n->getURI()->toString();
                string sub = s.substr( s.rfind("#")+1 );
                if( !sub.empty() && sub[0] >= '0' && sub[0] <= '9' && sub.length() > WORD_NODE_TEXT_OFFSET )
                    sub = sub.substr( WORD_NODE_TEXT_OFFSET );
    
                ret << sub;
                return ret.str();
            }

            fh_model getWordnetModel()
            {
                fh_context md = Shell::acquireContext("~/.ferris/wordnet/metadata");
                return Model::FromMetadataContext( md );
            }
            
        };

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        struct tryToGetUUIDNode_StringARM
        {
            static void set_tryToGetImplicitTreeSmushHasFailed_forURL( fh_context c, bool v )
                {
                    c->m_tryToGetImplicitTreeSmushHasFailed_forURL = v;
                }
            static bool get_tryToGetImplicitTreeSmushHasFailed_forURL( fh_context c )
                {
                    return c->m_tryToGetImplicitTreeSmushHasFailed_forURL;
                }
        };

        /****************************************/
        /****************************************/
        /****************************************/

        class TransitiveRDFNodeCopy
        {
            fh_model m_outmodel;
            fh_model m_inmodel;
            fh_node m_inRootNode;
            fh_node m_newRootSubject;
            bool m_OverwriteExisting;
            bool m_createNewBNodes;
            
            typedef map< fh_node, fh_node, fh_node_compare > m_bnodeMapping_t;
            m_bnodeMapping_t m_bnodeMapping;
            
        public:
            TransitiveRDFNodeCopy( fh_model outmodel,
                                   fh_model inmodel,
                                   fh_node n,
                                   bool clone = true )
                :
                m_outmodel( outmodel ),
                m_inmodel( inmodel ),
                m_inRootNode( n ),
                m_newRootSubject( 0 ),
                m_OverwriteExisting( true ),
                m_createNewBNodes( false )
                {
                    if( !clone )
                    {
                        m_OverwriteExisting = false;
                        m_createNewBNodes = true;
                    }
                }

            void setNewRootSubject( fh_node n )
                {
                    m_newRootSubject = n;
                }
            void setOverwriteExisting( bool v )
                {
                    m_OverwriteExisting = v;
                }
            void setCreateNewBNodes( bool v )
                {
                    m_createNewBNodes = v;
                }

            void insertAllObjects( fh_node n, fh_node newRootSubject )
                {
                    LG_RDF_D << "TransitiveRDFNodeCopy::insertAllObjects(begin) "
                             << " n:" << n->toString()
                             << endl;
                    
                    NodeIterator end   = NodeIterator();
                    NodeIterator piter = m_inmodel->getArcsOut( n );
                    for( ; piter != end; ++piter )
                    {
                        LG_RDF_D << "TransitiveRDFNodeCopy::insertAllObjects() "
                                 << " pred:" << (*piter)->toString()
                                 << endl;
                        
                        NodeIterator oiter = m_inmodel->findObjects( n, *piter );
                        for( ; oiter != end; ++oiter )
                        {
                            LG_RDF_D << "TransitiveRDFNodeCopy::insertAllObjects() "
                                     << " obj:" << (*oiter)->toString()
                                     << endl;
                            
                            fh_statement st = new Statement( n, *piter, *oiter );
//                            if( !m_outmodel->contains( st ) )
                            {
                                LG_RDF_D << "TransitiveRDFNodeCopy::insertAllObjects(making) "
                                         << " m_createNewBNodes:" << m_createNewBNodes
                                         << " m_OverwriteExisting:" << m_OverwriteExisting
                                         << " newRootSubject:" << toVoid(newRootSubject)
                                         << " obj:" << (*oiter)->toString()
                                         << endl;
                                if( m_createNewBNodes )
                                {
                                    fh_node o = st->getObject();
                                    if( o->isBlank() )
                                    {
                                        fh_node b = m_outmodel->CreateBlankNode();
                                        st->setObject( b );
                                        m_bnodeMapping[ o ] = b;
                                    }
                                    fh_node s = st->getSubject();
                                    if( s->isBlank() )
                                    {
                                        fh_node b = m_bnodeMapping[ s ];
                                        st->setSubject( b );
                                    }
                                }
                                if( newRootSubject )
                                    st->setSubject( newRootSubject );

                                if( m_OverwriteExisting
                                    || !m_outmodel->getObject( st->getSubject(), st->getPredicate() ) )
                                {
                                    m_outmodel->insert( st );
                                }

                            }
                            LG_RDF_D << "TransitiveRDFNodeCopy::insertAllObjects(recurse) "
                                     << " object-becoming-node:" << (*oiter)->toString()
                                     << endl;
                            insertAllObjects( *oiter, 0 );
                        }
                    }

                    LG_RDF_D << "TransitiveRDFNodeCopy::insertAllObjects(end) "
                             << " n:" << n->toString()
                             << endl;
                }
            void operator()()
                {
                    insertAllObjects( m_inRootNode, m_newRootSubject );
                }
        };
        
//         typedef map< fh_node, fh_node, fh_node_compare > bnodeMapping_t;
//         bnodeMapping_t bnodeMapping;
//         static void
//         insertAllObjects( fh_model outmodel,
//                           fh_model inmodel,
//                           fh_node n,
//                           fh_node newRootSubject,
//                           bool dontSetExisting,
//                           bnodeMapping_t& bnodeMapping,
//                           bool createNewBNodes )
//         {
//             NodeIterator end   = NodeIterator();
//             NodeIterator piter = inmodel->getArcsOut( n );
//             for( ; piter != end; ++piter )
//             {
//                 NodeIterator oiter = inmodel->findObjects( n, *piter );
//                 for( ; oiter != end; ++oiter )
//                 {
//                     fh_statement st = new Statement( n, *piter, *oiter );
//                     if( !outmodel->contains( st ) )
//                     {
//                         if( createNewBNodes )
//                         {
//                             fh_node o = st->getObject();
//                             if( o->isBlank() )
//                             {
//                                 fh_node b = Node::CreateBlankNode();
//                                 st->setObject( b );
//                                 bnodeMapping[ o ] = b;
//                             }
//                             fh_node s = st->getSubject();
//                             if( s->isBlank() )
//                             {
//                                 fh_node b = bnodeMapping[ s ];
//                                 st->setSubject( b );
//                             }
//                         }
//                         if( newRootSubject )
//                             st->setSubject( newRootSubject );

//                         if( !dontSetExisting || !outmodel->getObject( st->getSubject(), st->getPredicate() ) )
//                             outmodel->insert( st );
                        
//                         insertAllObjects( outmodel, inmodel, *oiter, 0, dontSetExisting, bnodeMapping, createNewBNodes );
//                     }
//                 }
//             }
//         }
        /*
         * Use 'n' as a subject node and insert all of its transitive triples
         * into the output model 'outmodel'. Nodes are copied from 'inmodel'
         */
        static void
        insertAllObjects( fh_model outmodel, fh_model inmodel, fh_node n )
        {
            bool clone = true;
            TransitiveRDFNodeCopy trdf_copy( outmodel, inmodel, n, clone );
            trdf_copy();
        }

        /******************************/
        /******************************/
        /******************************/
        

        static const std::string ATTR_PREFIX =
        "http://witme.sf.net/libferris.web/rdf/ferris-attr/";
        static const std::string SCHEMA_PREFIX =
        "http://witme.sf.net/libferris.web/rdf/ferris-schema/";
        

        static std::string getURI( const std::string& baseuri,
                                   const std::string& ferrisEAName,
                                   const std::string& pfx = "" )
        {
            std::string ret = baseuri;
            if( !pfx.empty() && starts_with( ferrisEAName, pfx ) )
                ret += ferrisEAName.substr( pfx.length() );
            else
                ret += ferrisEAName;

            return ret;
        }

        std::string getAttrPrefix()
        {
            return ATTR_PREFIX;
        }
        
        
        std::string getPredicateURI( const std::string& rdn )
        {
            if( starts_with( rdn, "schema:" ))
            {
                return getURI( SCHEMA_PREFIX, rdn, "schema:" );
            }
            return getURI( ATTR_PREFIX, rdn );
        }

        std::string stripPredicateURIPrefix( const std::string& pred )
        {
            string ret = pred;
            
            PrefixTrimmer trimmer;
            trimmer.push_back( ATTR_PREFIX );
            trimmer.push_back( SCHEMA_PREFIX );
            ret = trimmer( ret );

            return ret;
        }

        void myrdfSmush( fh_context existingc,
                         fh_context oldc,
                         bool unifyUUIDNodes )
        {
            return myrdfSmush( existingc, oldc->getURL(), unifyUUIDNodes );
        }
        
        void myrdfSmush( fh_context existingc,
                         const std::string& oldpath_maybe_relative,
                         bool unifyUUIDNodes )
        {
            string oldearl = oldpath_maybe_relative;
            if( oldearl.empty() )
            {
                stringstream ss;
                ss << "No old URL given for existingc:" << existingc->getURL() << endl;
                Throw_NoSuchObject( tostr(ss), 0 );
            }
            if( string::npos == oldearl.find("/") )
            {
                string existing_path = existingc->getURL();
                int epos = existing_path.rfind("/");
                existing_path = existing_path.substr( 0, epos );
                existing_path += "/" + oldearl;
                oldearl = existing_path;
            }
            else if( oldearl[0] == '.'  )
            {
                stringlist_t el = Util::parseSeperatedList( existingc->getURL(), '/' );
                stringlist_t ol = Util::parseSeperatedList( oldearl, '/' );
                for( stringlist_t::const_iterator oiter = ol.begin();
                     oiter != ol.end(); ++oiter )
                {
                    if( *oiter == ".." )
                    {
                        stringlist_t::iterator tmp = el.end();
                        --tmp;
                        el.erase( tmp );
                    }
                    else if( *oiter == "." )
                    {
                    }
                    else
                    {
                        el.push_back( *oiter );
                    }
                }

                stringstream ss;
                for( stringlist_t::const_iterator eiter = el.begin();
                     eiter != el.end(); ++eiter )
                {
                    ss << *eiter;
                }

                oldearl = tostr(ss);
            }

            LG_RDF_D << "smushing oldearl:" << oldearl << " existingc:" << existingc->getURL() << endl;
            
            fh_model m = getDefaultFerrisModel();
            fh_node oldURI = Node::CreateURI( oldearl );
            fh_node newURI = Node::CreateURI( existingc->getURL() );

            if( unifyUUIDNodes )
            {
                if( fh_node old_uuidnode = tryToGetUUIDNode( existingc ) )
                {
                    m->erase( Node::CreateURI( existingc->getURL() ),
                              uuidPredNode(),
                              old_uuidnode );
                }
                
                fh_statement partial_statement = new Statement();
                partial_statement->setSubject( oldURI );
                StatementIterator si = m->findStatements( partial_statement );
                for( StatementIterator si_end; si != si_end; ++si )
                {
                    fh_node pred = (*si)->getPredicate();
                    fh_node obj  = (*si)->getObject();
                    
                    m->insert( newURI, pred, obj );
                }
            }
            else
            {
                fh_node n = m->getObject( oldURI, uuidPredNode() );
                if( n )
                {
                    fh_node uuidnode = ensureUUIDNode( existingc );

                    LG_RDF_D << "myrdfSmush()" << endl;
                    LG_RDF_D << "old uuidnode:" << n->toString() << endl;
                    LG_RDF_D << "new uuidnode:" << uuidnode->toString() << endl;
                    
                    bool clone = false;
                    TransitiveRDFNodeCopy trdf_copy( m,m, n, clone );
                    trdf_copy.setNewRootSubject( uuidnode );
                    trdf_copy();
                }
            }
            


            
//             stringlist_t rdf_ea_sl = Util::parseCommaSeperatedList(
//                 getStrAttr( oldchild, "rdf-ea-names", "" ) );
//             stringmap_t rdf_ea_map;
//             if( !rdf_ea_sl.empty() )
//             {
//                 stringlist_t::const_iterator end = rdf_ea_sl.end();
//                 for( stringlist_t::const_iterator si = rdf_ea_sl.begin(); si!=end; ++si )
//                 {
//                     rdf_ea_map[ *si ] = getStrAttr( oldchild, *si, "", true, true );
//                 }
//             }

//             if( !rdf_ea_map.empty() )
//             {
//                 stringmap_t::const_iterator end = rdf_ea_map.end();
//                 for( stringmap_t::const_iterator mi = rdf_ea_map.begin(); mi!=end; ++mi )
//                 {
//                     setStrAttr( child, mi->first, mi->second, true, true );
//                 }
//             }
            
        }
        
        void myrdfSmushUnion( const stringlist_t& srcs )
        {
            fh_model m = getDefaultFerrisModel();

            typedef list< fh_node > nodes_t;
            nodes_t nodes;
            
            for( stringlist_t::const_iterator si = srcs.begin();
                 si != srcs.end(); ++si )
            {
                nodes.push_back( Node::CreateURI( *si ) );
            }

            for( nodes_t::iterator ni = nodes.begin();
                 ni != nodes.end(); ++ni )
            {
                fh_node n = *ni;

                for( nodes_t::iterator nj = nodes.begin();
                     nj != nodes.end(); ++nj )
                {
                    fh_node n2 = *nj;
                    if( n2 == n )
                        continue;

                    bool clone = false;
                    TransitiveRDFNodeCopy trdf_copy( m,m, n2, clone );
                    trdf_copy.setNewRootSubject( n );
                    trdf_copy();
                    

//                     fh_statement partial_statement = new Statement();
//                     partial_statement->setSubject( n2 );
//                     StatementIterator si = m->findStatements( partial_statement );
//                     for( StatementIterator si_end; si != si_end; ++si )
//                     {
//                         fh_node pred = (*si)->getPredicate();
//                         fh_node obj  = (*si)->getObject();

//                         if( !m->getObject( n, pred ) )
//                             m->insert( n, pred, obj );
//                     }
                    
                }
            }
        }

// COMMENT: sopranoea should use getPredicateURI() to prepend the URI part.
//         FERRISEXP_API const std::string& getRDFBaseURI();
//         const std::string& getRDFBaseURI()
//         {
//             return RDF::RDF_FERRIS_BASE;
//         }

        string regexEscape( const std::string& s )
        {
            stringstream iss;
            stringstream oss;
            iss << s;
            char ch;
            while( iss >> noskipws >> ch )
            {
                if( ( ch >= 'a' && ch <= 'z' ) 
                    || ( ch >= 'A' && ch <= 'Z' )
                    || ( ch >= '0' && ch <= '9' ) )
                {
                    oss << ch;
                }
                else
                {
                    oss << '\\' << ch;
                }
            }
            return oss.str();
        }


        fh_node getFastSmushByNameNode( fh_SmushSet smushSet, fh_context c )
        {
            stringstream ss;
            ss << smushSet->getName() << "-";
            ss << c->getDirName();
            fh_node subject = Node::CreateURI( getPredicateURI( tostr(ss) ) );
            return subject;
        }
        
        static fh_node createUUIDNode( fh_model m, fh_context c )
        {
            LG_RDF_D << "createUUIDNode(top) c:" << c->getURL() << endl;
            
            string  earl     = c->getURL();
            fh_node earlnode = Node::CreateURI( earl );
            string     uuid = Util::makeUUID();
            fh_node uuidnode = Node::CreateURI( getPredicateURI( uuid ) );
            LG_RDF_D << "createUUIDNode(2) c:" << c->getURL() << endl;
            // cerr << "createUUIDNode() adding..." << endl
            //      << "  subj:" << earlnode->toString()
            //      << "  pred:" << uuidPredNode()->toString()
            //      << "  obj:" << uuidnode->toString()
            //      << endl;
            m->insert( earlnode, uuidPredNode(), uuidnode );
            
            {
                fh_context p = c->getParent();
                
                if( fh_SmushSet smushSet = p->tryToGetImplicitTreeSmushSet() )
                {
//                         fh_rex rex = smushSet->getRegex();
//                         if( rex && regex_search( earl, rex, boost::match_any ) )
                    {
                        fh_node subject = getFastSmushByNameNode( smushSet, c );
                        m->insert( subject, smushCacheUUIDPredNode(), uuidnode );
                        LG_RDF_D << "Adding rdf smush cache node." << endl
                                 << " subject:" << subject->toString()
                                 << " pred:" << smushCacheUUIDPredNode()->toString()
                                 << " uuid:" << uuidnode->toString()
                                 << endl;
                    }
                }
            }
            
            LG_RDF_D << "createUUIDNode(3) c:" << c->getURL() << endl;
            fh_node ret = uuidnode;
//            fh_node ret = m->getObject( earlnode, uuidPredNode() );
            setUUIDNodeModificationTime( ret );
            LG_RDF_D << "createUUIDNode(4) c:" << c->getURL() << endl;
            return ret;
        }
        
        fh_node tryToGetUUIDNode( fh_context c )
        {
            fh_model m = getDefaultFerrisModel();

            fh_node earlnode = Node::CreateURI( c->getURL() );
            fh_node uuidnode = m->getObject( earlnode, uuidPredNode() );

            if( uuidnode )
                LG_RDF_D << "tryToGetUUIDNode exists:" << uuidnode->toString()
                         << endl;
            
            // 
            // If the user has implicit tree smushes setup then we should see if there
            // is a uuidnode attached with the same filename in one of the other paths
            // in a smushset. If we find such a node then we automatically associate
            // it with the current file.
            //
            if( !uuidnode && c->isParentBound() )
            {
                fh_context p = c->getParent();

                LG_RDF_D << "tryToGetUUIDNode() for c:" << c->getURL() << endl;
                 
                if( !tryToGetUUIDNode_StringARM::get_tryToGetImplicitTreeSmushHasFailed_forURL( c ) )
                {
                    tryToGetUUIDNode_StringARM::set_tryToGetImplicitTreeSmushHasFailed_forURL( c, true );

                    if( fh_SmushSet smushSet = p->tryToGetImplicitTreeSmushSet() )
                    {
                    //
                    // A smush set group leader concept. If the regex is matched then we don't
                    // do any smushing for this URL. This is handy for example when there is
                    // an ingress filesystem where new files are always created (like copied
                    // from a digital camera to there) and there is never any need to smush.
                    //
                    static const string smushLeaderRegexString = getEDBString( FDB_GENERAL,
                                                                               CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_K,
                                                                               CFG_RDF_GLOBAL_SMUSH_GROUP_LEADER_DEFAULT );
                    if( !smushLeaderRegexString.empty() )
                    {
                        static const fh_rex rex = toregexh( smushLeaderRegexString );
                        if( regex_search( p->getURL(), rex, boost::match_any ) )
                        {
                            LG_RDF_D << "tryToGetUUIDNode() group leader regex matches. Not doing any smushing."
                                     << " smushLeaderRegexString:" << smushLeaderRegexString
                                     << " for c:" << c->getURL() << endl;
                            if( uuidnode )
                                return uuidnode;
                            return createUUIDNode( m, c );
                        }
                    }
                        
                        //
                        // try to hit the new cache directly
                        //
                        {
                            fh_node subject = getFastSmushByNameNode( smushSet, c );
                            LG_RDF_D << "Checking rdf smush cache node." << endl
                                     << " subject:" << subject->toString()
                                     << " pred:" << smushCacheUUIDPredNode()->toString()
                                     << endl;
                            fh_node uuidnode = m->getObject( subject, smushCacheUUIDPredNode() );
                            if( uuidnode )
                            {
                                LG_RDF_D << "FAST Linking uuid:" << uuidnode->toString()
                                         << " to c:" << c->getURL() << endl;
                                m->insert( earlnode, uuidPredNode(), uuidnode );
                                setUUIDNodeModificationTime( uuidnode );
                                return uuidnode;
                            }
                        }
                        
                        
                        
//                        cerr << "tryToGetUUIDNode() implicit smushing for c:" << c->getURL() << endl;
                        
                        stringstream queryiss;
                        queryiss << "PREFIX ferris: <http://witme.sf.net/libferris.web/rdf/ferris-attr/>\n"
                                 << endl
                                 << "SELECT ?uuid ?earl" << endl
                                 << "  WHERE {  " << endl
                                 << "     ?earl ferris:uuid ?uuid . " << endl
                                 << "     FILTER ( " << endl;

                        string regex_string = smushSet->getRegexString();
                        string regexq_filename = Util::replace_all( regexEscape( c->getDirName() ), '\\', "\\\\" );
                        queryiss << "       regex( str(?earl), "
                                 << " \"" << regex_string << "(.*/)*" << regexq_filename << "$\") " << endl;
//                         stringlist_t sl;
//                         const SmushSet::m_smushes_t ss = smushSet->getSmushes();
//                         for( rs<SmushSet::m_smushes_t> r( ss ); r; ++r )
//                         {
//                             sl.push_back( si->first );
//                         }
//                         std::string compregex = MakeCompositeRegexString( sl );

                        
//                         bool virgin = true;
//                         const SmushSet::m_smushes_t ss = smushSet->getSmushes();
//                         SmushSet::m_smushes_t::const_iterator si = ss.begin();
//                         SmushSet::m_smushes_t::const_iterator se = ss.end();
//                         for( ; si != se ; ++si )
//                         {
//                             const string& regex_string = si->first;
//                             fh_regex r = si->second;
//                             string regexq_filename = Util::replace_all( regexEscape( c->getDirName() ), '\\', "\\\\" );

//                             if( !virgin )
//                                 queryiss << " || " ;
//                             virgin = false;
                        

//                             queryiss << "       regex( str(?earl), "
//                                      << " \"" << regex_string << "(.*/)*" << regexq_filename << "$\") " << endl;
//                         }

                        queryiss << " ) } LIMIT 1  " << endl;
                        
                        string query = queryiss.str();
                        LG_RDF_D << "Implicit Tree Smush query:" << endl << query << endl;

                        //
                        //
                        //
                        LG_RDF_W << "SPARQL is too slow to do at runtime. see apps/rdf/ferris-myrdf-inference" << endl;
//                         BindingsIterator iter = m->findBindings( query );
//                         BindingsIterator e;
//                         for( ; iter != e ; ++iter )
//                         {
//                             const BindingsIterator::binding& b = *iter;
//                             uuidnode = b.getByName( "uuid" );
//                             //
//                             // attach the found uuidnode with the URL.
//                             //
//                             LG_RDF_D << "Linking uuid:" << uuidnode->toString()
//                                      << " to c:" << c->getURL() << endl;
//                             m->insert( earlnode, uuidPredNode(), uuidnode );
//                             setUUIDNodeModificationTime( uuidnode );
//                             return uuidnode;
//                         }

                        // A once off hit making sure that all directories in a tree smush
                        // will have a uuid node.
                        LG_RDF_D << "tryToGetUUIDNode() calling ensure for c:" << c->getURL() << endl;
                        return ensureUUIDNode( c );
                    }
                }
            }

            return uuidnode;
        }

        fh_node ensureUUIDNode( fh_model m, fh_context c )
        {
            fh_node uuidnode = tryToGetUUIDNode( c );
            if( uuidnode )
                return uuidnode;

            LG_RDF_D << "ensureUUIDNode() creating for c:" << c->getURL() << endl;
            return createUUIDNode( m, c );
        }


        fh_node ensureUUIDNode( fh_context c )
        {
            fh_model m = getDefaultFerrisModel();
            return ensureUUIDNode( m, c );
        }

        
        fh_node& uuidMTimeNode()
        {
            static fh_node ret = 0;
            if( !ret )
                ret = Node::CreateURI( getPredicateURI( "mtime" ) );
            return ret;
        }
        
        time_t getUUIDNodeModificationTime( ::Ferris::RDFCore::fh_node n )
        {
            fh_model m = getDefaultFerrisModel();
            fh_node pred = uuidMTimeNode();
            fh_node obj = m->getObject( n, pred );
            if( obj )
            {
                time_t ret = toType<time_t>(obj->toString());
                LG_RDF_D << "getUUIDNodeModificationTime() ret:" << ret
                         << " for:" << n->toString() << endl;
                return ret;
            }
            else
            {
                LG_RDF_D << "getUUIDNodeModificationTime() no data for:" << n->toString() << endl;
            }
            return 0;
        }
        
        void setUUIDNodeModificationTime( ::Ferris::RDFCore::fh_node n, time_t t )
        {
            fh_model m = getDefaultFerrisModel();
            if( !t )
            {
                t = Time::getTime();
            }

            LG_RDF_D << "setUUIDNodeModificationTime() n:" << n->toString()
                     << " t:" << t << endl;
//             cerr << "setUUIDNodeModificationTime() n:" << n->toString()
//                  << " t:" << t << endl;
//             BackTrace();
            
            fh_node pred = uuidMTimeNode();
            LG_RDF_D << "setUUIDNodeModificationTime() 2" << endl;
            fh_node obj = Node::CreateLiteral( tostr(t) );
            LG_RDF_D << "setUUIDNodeModificationTime() 3" << endl;
            LG_RDF_D << "setUUIDNodeModificationTime() n:" << n->toString() << endl;
            LG_RDF_D << "setUUIDNodeModificationTime() pred:" << pred->toString() << endl;
            LG_RDF_D << "setUUIDNodeModificationTime() obj:" << obj->toString() << endl;
            m->set( n, pred, obj );
            LG_RDF_D << "setUUIDNodeModificationTime() 4" << endl;
            m->sync();
            LG_RDF_D << "setUUIDNodeModificationTime() 5" << endl;
        }

        void setSopranoEASubjectNode( ::Ferris::RDFCore::fh_node n, time_t t )
        {
            fh_model m = getDefaultFerrisModel();

            fh_node uuidnode = m->getSubject( uuidOutOfBandPredNode(), n );
            if( uuidnode )
                setUUIDNodeModificationTime( uuidnode, t );
        }
        
        
        
        
        fh_node tryToGetSopranoEASubjectNode( fh_context c )
        {
            LG_RDF_D << "tryToGetSopranoEASubjectNode(top) c:" << c->getURL() << endl;
            
            fh_model m = getDefaultFerrisModel();
            fh_node ret = tryToGetUUIDNode( c );
            LG_RDF_D << "tryToGetSopranoEASubjectNode(2) c:" << c->getURL() << endl;
            if( ret )
            {
                ret = m->getObject( ret, uuidOutOfBandPredNode() );
                LG_RDF_D << "tryToGetSopranoEASubjectNode(have node) c:" << c->getURL() << endl;
            }
            return ret;
        }
        
        fh_node ensureSopranoEASubjectNode( fh_context c )
        {
            fh_node ret = tryToGetSopranoEASubjectNode( c );
            if( ret )
            {
                LG_RDF_D << "ensureSopranoEASubjectNode(exists) ret:" << ret->toString() << endl;
                return ret;
            }
            
            fh_model m = getDefaultFerrisModel();
            fh_node uuidnode = ensureUUIDNode( c );
//            fh_node mdnode = Node::CreateURI( getPredicateURI( Util::makeUUID() ) );
            fh_node mdnode = m->CreateBlankNode();
            m->set( uuidnode, uuidOutOfBandPredNode(), mdnode );
            setUUIDNodeModificationTime( uuidnode );
            LG_RDF_D << "ensureSopranoEASubjectNode() uuidnode:" << uuidnode->toString() << endl;
            LG_RDF_D << "ensureSopranoEASubjectNode() ret:" << mdnode->toString() << endl;
            return mdnode;
        }
        
        

        fh_node& uuidPredNode()
        {
            static fh_node ret = 0;
            if( !ret )
                ret = Node::CreateURI( getPredicateURI( "uuid" ) );
            return ret;
        }

        fh_node& smushCacheUUIDPredNode()
        {
            static fh_node ret = 0;
            if( !ret )
                ret = Node::CreateURI( getPredicateURI( "smushcacheuuid" ) );
            return ret;
        }

        fh_node& uuidOutOfBandPredNode()
        {
            static fh_node ret = 0;
            if( !ret )
                ret = Node::CreateURI( getPredicateURI( "out-of-band-ea" ) );
            return ret;
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        SmushSet::~SmushSet()
        {
        }
            

        const std::string&
        SmushSet::getName() const
        {
            return m_name;
        }
        
        void
        SmushSet::setName( const std::string& n )
        {
            m_treeSmushing->updateSmushName( m_name, n );
            m_name = n;
        }
        

        const SmushSet::m_smushes_t&
        SmushSet::getSmushes() const
        {
            return m_smushes;
        }

        const string&
        SmushSet::getRegexString()
        {
            return m_regexString;
        }

        fh_rex
        SmushSet::getRegex()
        {
            if( !m_rex )
            {
                if( !getRegexString().empty() )
                {
                    m_rex = toregexh( getRegexString() );
                }
            }
            return m_rex;
        }
        
        
        void
        SmushSet::setSmushes( m_smushes_t& v )
        {
            m_smushes = v;
            stringlist_t sl;
            for( rs<m_smushes_t> r(m_smushes); r; ++r )
                sl.push_back( r->first );
            m_regexString = MakeCompositeRegexString( sl );
        }
        
        SmushSet::SmushSet( fh_TreeSmushing p, const std::string& name )
            :
            m_treeSmushing( p ),
            m_name( name ),
            m_rex( 0 )
        {
        }
        
            
        TreeSmushing::TreeSmushing()
            :
            m_shouldSave( false )
        {
        }
        
        TreeSmushing::~TreeSmushing()
        {
        }

        const TreeSmushing::m_smushSets_t&
        TreeSmushing::getAll() const
        {
            return m_smushSets;
        }
        
        
        fh_SmushSet
        TreeSmushing::getSmush( const std::string& n ) const
        {
            m_smushSets_t::const_iterator ci = m_smushSets.find( n );
            if( ci != m_smushSets.end() )
                return ci->second;
            return 0;
        }
        
        void
        TreeSmushing::swap( fh_TreeSmushing other )
        {
            m_smushSets.swap( other->m_smushSets );
        }
        
        fh_SmushSet
        TreeSmushing::newSmush( const std::string& name )
        {
            m_smushSets[ name ] = new SmushSet( this, name );
            return m_smushSets[ name ];
        }

        std::string
        TreeSmushing::getBoostSerializePath()
        {
            return CleanupURL( "~/.ferris/tree-smushing.boost" );
        }

        template< class ArchiveClassT >
        void
        TreeSmushing::sync( ArchiveClassT& archive )
        {
            m_smushSets_t::iterator ci = m_smushSets.begin();
            m_smushSets_t::iterator  e = m_smushSets.end();

            long sz = m_smushSets.size();
            archive << sz;
            for( ; ci != e ; ++ci )
            {
                archive << ci->first;

                fh_SmushSet ss = ci->second;
                long sz = ss->m_smushes.size();
                archive << sz;

                SmushSet::m_smushes_t::iterator si = ss->m_smushes.begin();
                SmushSet::m_smushes_t::iterator se = ss->m_smushes.end();
                for( ; si != se; ++si )
                {
                    archive << si->first;
                    const Regex& rr = *(GetImpl(si->second));
                    archive << rr;
                }
            }
        }
        
        void
        TreeSmushing::sync()
        {
            if( m_shouldSave )
            {
                {
                    string fname = getBoostSerializePath() + ".txt";
                    std::ofstream ofs( fname.c_str() );
                    boost::archive::text_oarchive archive( ofs );
                    sync( archive );
                }
                
                {
                    std::ofstream ofs( getBoostSerializePath().c_str() );
                    boost::archive::binary_oarchive archive( ofs );
                    sync( archive );
                }
            }
        }
        

        template<class Archive>
        void
        TreeSmushing::load( Archive & archive )
        {
            long sz = 0;
            archive >> sz;
            for( long iter = 0; iter < sz; ++iter )
            {
                string name;
                archive >> name;
                fh_SmushSet ss = new SmushSet( this, name );
                long ss_sz = 0;
                archive >> ss_sz;
                        
                SmushSet::m_smushes_t smushes;
                for( long ss_iter = 0; ss_iter < ss_sz; ++ss_iter )
                {
                    string s;
                    archive >> s;
                             
                    fh_regex r = new Regex("");
                    Regex& rr = *(GetImpl(r));
                    archive >> rr;
                    smushes.insert( make_pair( s, r ) );
                }
                ss->setSmushes( smushes );
                m_smushSets.insert( make_pair( name, ss ) );
            }
        }
        
        
        TreeSmushing::TreeSmushing( bool shouldSave )
            :
            m_shouldSave( shouldSave )
        {
            if( m_shouldSave )
            {
//                if( !access( getBoostSerializePath().c_str(), R_OK ) )
                {
                    try
                    {
                        std::ifstream ifs( getBoostSerializePath().c_str() );
                        boost::archive::binary_iarchive archive( ifs );
                        load( archive );
                    }
                    catch( exception&e )
                    {
                        string fname = getBoostSerializePath() + ".txt";
                        std::ifstream ifs( fname.c_str() );
                        boost::archive::text_iarchive archive( ifs );
                        load( archive );
                    }
                    
                }
            }
        }
        
        void
        TreeSmushing::updateSmushName( const std::string& oldname, const std::string& newname )
        {
            m_smushSets_t::iterator ci = m_smushSets.find( oldname );
            if( ci != m_smushSets.end() )
            {
                fh_SmushSet d = ci->second;
                m_smushSets.erase( ci );
                m_smushSets[ newname ] = d;
            }
        }
        
        
        fh_TreeSmushing getDefaultImplicitTreeSmushing()
        {
            static fh_TreeSmushing ret = 0;
            if( !ret )
            {
                ret = new TreeSmushing( true );
            }
            return ret;
        }

    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    using namespace ::Ferris::Semantic;
    
    Semantic::fh_SmushSet
    Context::tryToGetImplicitTreeSmushSet()
    {
        if( m_tryToGetImplicitTreeSmushHasFailed_forDirectory )
            return 0;

        string earl = getURL();
        earl += "/";
        fh_TreeSmushing ts = getDefaultImplicitTreeSmushing();
        const TreeSmushing::m_smushSets_t& a = ts->getAll();
        TreeSmushing::m_smushSets_t::const_iterator ai = a.begin();
        TreeSmushing::m_smushSets_t::const_iterator ae = a.end();

        LG_RDF_D << "Context::tryToGetImplicitTreeSmush() earl:" << earl << endl;
        
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

                LG_RDF_D << "Context::tryToGetImplicitTreeSmush() n:" << n << endl;

                if( (*r)( earl ) )
                {
                    return ret;
                }
            }
        }

        m_tryToGetImplicitTreeSmushHasFailed_forDirectory = true;
        return 0;
    }

    
};
