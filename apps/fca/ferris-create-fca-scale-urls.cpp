/******************************************************************************
*******************************************************************************
*******************************************************************************

    create a scale from the URLs in the EA index
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

    $Id: ferris-create-fca-scale-urls.cpp,v 1.8 2010/09/24 21:31:10 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * return 0 for success
 * return 1 for generic error
*/

#include "libferrisfcascaling.hh"

#include <Ferris/FerrisSemantic.hh>
#include <Ferris/FerrisBoost.hh>

using namespace Ferris::RDFCore;
using namespace Ferris::Semantic::Wordnet;
using namespace Ferris::Semantic;

const string PROGRAM_NAME = "ferris-create-fca-scale-urls";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

/********************************************************************************/
/********************************************************************************/
/********************************************************************************/

typedef map< string, stringmap_t::const_iterator > matches_t;
typedef map< string, matches_t > hyponyms_t;

fh_node hasWord = Node::CreateURI( RDFSCHEMA + "hasWord" );
fh_node NounSynSet = Node::CreateURI( RDFSCHEMA + "NounSynSet" );
fh_node Type = Node::CreateURI( "http://www.w3.org/1999/02/22-rdf-syntax-ns#type" );
fh_node hasGloss = Node::CreateURI( RDFSCHEMA + "hasGloss" );
fh_node partMeronymOf = Node::CreateURI( RDFSCHEMA + "partMeronymOf" );

void getHyponyms( fh_model m, hyponyms_t& h, stringmap_t::const_iterator dni )
{
    string word = dni->first;

    nodelist_t synlist;
    getSynSet( m, synlist, word );
    NodeIterator end   = NodeIterator();
    
    for( nodelist_t::const_iterator syniter = synlist.begin(); syniter != synlist.end(); ++syniter )
    {
        fh_node synset = *syniter;

        // hypern
        {
            NodeIterator hiter = m->findObjects( synset, hyponymOf() );
            for( ; hiter != end; ++hiter )
            {
                fh_node hs = *hiter;

                h[ hs->getURI()->toString() ].insert( make_pair( word, dni ) );
                
//                 for( NodeIterator i = m->findSubjects( hasSynSet(), hs ); i != end; ++i )
//                 {
//                     fh_node syn = *i;
//                     string w = getWordNodeAsString( syn );
//                     h[ syn->getURI()->toString() ].insert( make_pair( word, dni ) );
//                 }
            }
        }
    }
        
        
//     NodeIterator end   = NodeIterator();
//     NodeIterator oiter = m->findSubjects( wordForm, wordNode );
//     for( ; oiter != end; ++oiter )
//     {
//         fh_node subject = *oiter;
// //        cerr << "have subject:" << subject->toString() << endl;

//         fh_node wordtype = m->getObject( subject, wordType );
// //        cerr << "have word type:" << wordtype->toString() << endl;
//         if( wordtype == noun )
//         {
//             NodeIterator hypiter = m->findObjects( subject, hyponymOf() );
//             for( ; hypiter != end; ++hypiter )
//             {
//                 fh_node hyp = *hypiter;
// //                cerr << "  hyp:" << hyp->toString() << endl;
// //                h[ word ].push_back( hyp->toString() );
//                 fh_uri uri = hyp->getURI();
//                 h[ uri->toString() ].insert( make_pair( word, dni ) );
//             }
//         }
//     }
}

string getAttributeName( fh_model m, const std::string& hypuri )
{
    NodeIterator end   = NodeIterator();
    stringstream retss;
    
//  [http://www.dcc.uchile.cl/~agraves/wordnet/1.0/schema#hasSynSet], [http://www.dcc.uchile.cl/~agraves/wordnet/1.0/synsets#102073042]}

    bool v = true;
    fh_node hs = Node::CreateURI( hypuri );
    for( NodeIterator i = m->findSubjects( hasSynSet(), hs ); i != end; ++i )
    {
        fh_node node = *i;
        if( v ) v = false;
        else    retss << "-";
        retss << cleanAttributeName( getWordNodeAsString( node ) );
    }

    cerr << "getAttributeName() hyp:" << hypuri << endl
         << "   ret:" << tostr(retss) << endl;
    
    
//     fh_node subj = Node::CreateURI( hypuri );
//     NodeIterator end   = NodeIterator();
//     NodeIterator iter = m->findObjects( subj, wordForm );
//     bool v = true;
//     for( ; iter != end; ++iter )
//     {
//         fh_node obj = *iter;
//         if( v ) v = false;
//         else    retss << "-";
        
//         retss << cleanAttributeName(obj->toString());
//     }

//     cerr << "getAttributeName() hypuri:" << hypuri
//          << " ret:" << retss.str()
//          << endl;
    
    return retss.str();
}


int main( int argc, char** argv )
{
    int exit_status = 0;
    
    try
    {
        unsigned long Verbose           = 0;
        unsigned long PathComponentsToStripFromEnd = 1;
        unsigned long MinMatchLength = 2;
        unsigned long UseWordnet     = 0;
        unsigned long RemoveVersionedDirectories = 0;
        const char*   findexPath_CSTR   = 0;

        struct poptOption optionsTable[] =
            {
                { "verbose", 'v', POPT_ARG_NONE, &Verbose, 0,
                  "report more details than normal", "" },

                { "index-path", 'P', POPT_ARG_STRING, &findexPath_CSTR, 0,
                  "path to existing postgresql EA index", "" },

                { "path-components-to-strip-from-end", 'E',
                  POPT_ARG_INT, &PathComponentsToStripFromEnd, 0,
                  "remove this amount of path components from the end of each url", "1" },

                { "min-match-length", 'm',
                  POPT_ARG_INT, &MinMatchLength, 0,
                  "min number of chars in URL match to make formal attribute", "2" },

                { "use-wordnet", 'W',
                  POPT_ARG_NONE, &UseWordnet, 0,
                  "Use wordnet to structure directory names", "(off)" },

                { "remove-versioned-directories", 'V',
                  POPT_ARG_NONE, &RemoveVersionedDirectories, 0,
                  "remove directories which contain explicit version numbers at the end", "(off)" },
                
                FERRIS_POPT_OPTIONS
                POPT_AUTOHELP
                POPT_TABLEEND
            };
        poptContext optCon;

        optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, (const char**)argv, optionsTable, 0);
        poptSetOtherOptionHelp(optCon, "[OPTIONS]* [-P ea-index-url] ");

        /* Now do options processing */
        int c=-1;
        while ((c = poptGetNextOpt(optCon)) >= 0)
        {}

        EAIndex::fh_idx idx = getEAIndex( findexPath_CSTR );
        string host   = idx->getConfig( CFG_IDX_HOST_K, CFG_IDX_HOST_DEF );
        string dbname = idx->getConfig( CFG_IDX_DBNAME_K, CFG_IDX_DBNAME_DEF );
        
        stringstream conSS;
        conSS << " host=" << host;
        conSS << " dbname=" << dbname;
        connection con( conSS.str() );
        work trans( con, "getting the schema..." );

        result res = trans.exec( "select * from urlmap" );

        stringmap_t dirNames;
        for (result::const_iterator c = res.begin(); c != res.end(); ++c)
        {
            string url = "";
            c[0].to(url);
            string s;
            stringstream urlss;

            for( int i=0; i<PathComponentsToStripFromEnd; ++i )
                url = url.substr( 0, url.rfind('/') );

            urlss << url;
            while( getline( urlss, s, '/' ) )
            {
                if( s.length() < MinMatchLength )
                    continue;

                string ffilter =  Util::replace_all( s, '+', "\\+" );
                s = cleanAttributeName(s);
                dirNames.insert( make_pair( s, ffilter ));
            }
        }

//        dirNames.erase( dirNames.find( "" ) );

        
        {
            stringset_t stopEndings;
            stopEndings.insert(".htmlcontent");

            boost::regex r = toregex( "[0-9]$" );
            
            stringmap_t::iterator dne = dirNames.end();
            for( stringmap_t::iterator dni = dirNames.begin();
                 dni != dne;  )
            {
                string       s = dni->first;
                string ffilter = dni->second;
                bool skip = false;
                
                if( RemoveVersionedDirectories && regex_match(s,r,boost::match_any) )
                {
                    stringmap_t::iterator t = dni;
                    ++dni;
                    dirNames.erase( t );
                    continue;
                }
                for( stringset_t::const_iterator si = stopEndings.begin();
                     si != stopEndings.end(); ++si )
                {
                    string ending = *si;
                    
                    if( ends_with( s, ending ) )
                    {
                        stringmap_t::iterator t = dni;
                        ++dni;
                        dirNames.erase( t );
                        skip = true;
                        break;
                    }
                }
                if( skip )
                    continue;
                
                ++dni;
            }
        }

        
        
        
        if( UseWordnet )
        {
            hyponyms_t hyponyms;

            fh_model m = Wordnet::getWordnetModel();
            stringmap_t::const_iterator dne = dirNames.end();
            for( stringmap_t::const_iterator dni = dirNames.begin(); dni != dne;  )
            {
                string       s = dni->first;
                string ffilter = dni->second;

//                cerr << "Getting HYP for s:" << s << endl;
                getHyponyms( m, hyponyms, dni );
                ++dni;
            }

            string wn_attr_prefix = "wn_";
            for( hyponyms_t::const_iterator hi = hyponyms.begin(); hi!=hyponyms.end(); ++hi )
            {
                const string&     hypuri = hi->first;
                const matches_t& matches = hi->second;

                if( matches.size() > 1 )
                {
                    string attr = getAttributeName( m, hypuri );
                    
                    fh_stringstream ffilterss;
                    ffilterss << "'(|";
//                    cerr << "sz:" << matches.size() << " hyp:" << hypuri << endl;
                    for( matches_t::const_iterator mi = matches.begin(); mi!=matches.end(); ++mi )
                    {
                        string dirName = mi->first;
//                        cerr << "    matches:" << dirName << endl;
                        const std::string& ffilter = mi->second->second;
                        ffilterss << "(url=~\\/" << ffilter << "\\/)";
                    }
                    ffilterss << ")'";
//                     cerr << "ADDING... attr:" << attr
//                          << " ffilter:" << tostr(ffilterss)
//                          << endl;
                    dirNames.insert( make_pair( wn_attr_prefix + attr, tostr(ffilterss) ));
                }
            }
        }
        
//        copy( dirNames.begin(), dirNames.end(),  ostream_iterator<string>( cerr,"\n") );
        stringmap_t::const_iterator dne = dirNames.end();
        for( stringmap_t::const_iterator dni = dirNames.begin();
             dni != dne; ++dni )
        {
            string       s = dni->first;
            string ffilter = dni->second;
            if( !starts_with( ffilter, "'(|" ))
                cout << " " << s << " '(url=~\\/" << ffilter << "\\/)' " << endl;
            else
                cout << " " << s << " " << ffilter << endl;
        }
        
        
    }
    catch( exception& e )
    {
        LG_FCA_ER << PROGRAM_NAME << " cought exception:" << e.what() << endl;
        cerr << "cought error:" << e.what() << endl;
        exit(1);
    }
    return exit_status;
}


