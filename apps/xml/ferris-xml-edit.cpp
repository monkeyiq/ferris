/******************************************************************************
*******************************************************************************
*******************************************************************************

    ferris ls
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

    $Id: ferris-xml-edit.cpp,v 1.3 2010/09/24 21:31:20 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <string>
#include <list>
#include <set>
#include <vector>
#include <algorithm>
#include <iomanip>

#include <Ferris.hh>
#include <FerrisDOM.hh>
#include <FerrisBoost.hh>
#include <Resolver_private.hh>
#include <Ferrisls.hh>
#include <popt.h>
#include <unistd.h>

using namespace std;
using namespace Ferris;
using namespace Ferris::XML;


const string PROGRAM_NAME = "ferris-xml-edit";

void usage(poptContext optCon, int exitcode, char *error, char *addl)
{
    
    poptPrintUsage(optCon, stderr, 0);
    if (error) fprintf(stderr, "%s: %s0", error, addl);
    exit(exitcode);
}

string commandLine = "";

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

stringmap_t UniqueNodeAttrs;
typedef multiset< pair< pair< string, string >, string > > EncounteredUniqueNodeAttrs_t;
EncounteredUniqueNodeAttrs_t EncounteredUniqueNodeAttrs;

void PerformUniqueNodeAttrs( fh_domdoc dom, DOMElement* element )
{
//    cerr << "PerformUniqueNodeAttrs()" << endl;
    string en = tostr( element->getNodeName() );
    stringmap_t::iterator smiter = UniqueNodeAttrs.find( en );
    if( smiter != UniqueNodeAttrs.end() )
    {
//        cerr << "en:" << en << endl;

        XMLCh* kx = XMLString::transcode( smiter->second.c_str() );
        bool isBound = element->getAttributeNode( kx );
        const XMLCh* vx = element->getAttribute( kx );
        XMLString::release( &kx );

        if( isBound )
        {
            
            string av = tostr(vx);
            EncounteredUniqueNodeAttrs_t::iterator eiter =
                EncounteredUniqueNodeAttrs.find( make_pair( *smiter, av ) );
            if( eiter != EncounteredUniqueNodeAttrs.end() )
            {
//                cerr << "av:" << av << " second:" << eiter->second << endl;
                if( av == eiter->second )
                {
//                    cerr << "DUPE" << endl;

                    if( DOMNode* p = element->getParentNode() )
                    {
                        p->removeChild( element );
                    }
                    return;
                }
            }
        
//            cerr << "  av:" << av << endl;
            EncounteredUniqueNodeAttrs.insert( make_pair( *smiter, av ));
        }
    }
    
    domnode_list_t nl;
    getChildren( nl, element );
    for( rs<domnode_list_t> ni( nl ); ni; ++ni )
    {
        if( (*ni)->getNodeType() == DOMNode::ELEMENT_NODE )
        {
            PerformUniqueNodeAttrs( dom, dynamic_cast<DOMElement*>(*ni) );
        }
    }
    
}




/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/
/********************************************************************************/


int main( int argc, const char** argv )
{
    const char* UniqueNodeAttrList_CSTR= "";
    unsigned long ShowVersion        = 0;

    struct poptOption optionsTable[] = {

//         { "show-columns", 0, POPT_ARG_STRING, &ShowColumnsCSTR, 0,
//           "Same as --show-ea",
//           "size,mime-type,name" },

        { "uniq-node-attr-list", 'U', POPT_ARG_STRING, &UniqueNodeAttrList_CSTR, 0,
          "Drop all but the first node with the given attribute. Format node1@attr,node2@attrx,...", 0 },

        { "version", 0, POPT_ARG_NONE, &ShowVersion, 0,
          "show version information and quit", 0 },
        
        /*
         * Standard Ferris options
         */
        FERRIS_POPT_OPTIONS
        
//         { "show-all-attributes", 0, POPT_ARG_NONE,
//           &ShowAllAttributes, 0,
//           "Show every available bit of metadata about each object in context",
//           0 },
        POPT_AUTOHELP
        POPT_TABLEEND
   };
    poptContext optCon;

    optCon = poptGetContext(PROGRAM_NAME.c_str(), argc, argv, optionsTable, 0);
    poptSetOtherOptionHelp(optCon, "[OPTIONS]* ...");

    if (argc < 1) {
//        poptPrintHelp(optCon, stderr, 0);
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    /* Now do options processing, get portname */
    int c=-1;
    while ((c = poptGetNextOpt(optCon)) >= 0)
    {
//         switch (c) {
//         }
    }

    if( ShowVersion )
    {
        cout << PROGRAM_NAME << " version: $Id: ferris-xml-edit.cpp,v 1.3 2010/09/24 21:31:20 ben Exp $\n"
             << "Written by Ben Martin, aka monkeyiq" << nl
             << nl
             << "Copyright (C) 2001-2006 Ben Martin" << nl
             << "This is free software; see the source for copying conditions.  There is NO\n"
             << "warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE."
             << endl;
        exit(0);
    }

    if( !UniqueNodeAttrList_CSTR )
    {
        poptPrintUsage(optCon, stderr, 0);
        exit(1);
    }

    try
    {
    
        string UniqueNodeAttrList = UniqueNodeAttrList_CSTR;
        stringlist_t nalist = Util::parseCommaSeperatedList( UniqueNodeAttrList );
        for( stringlist_t::iterator si = nalist.begin(); si != nalist.end(); ++si )
        {
            static boost::regex atrex("^([^@]+)@(.*)$");
            boost::smatch matches;
            if(boost::regex_match( *si, matches, atrex ))
            {
                cerr << "matches.size:" << matches.size() << endl;
                if( matches.size() == 3 )
                {
                    string n = matches[1];
                    string a = matches[2];
                    UniqueNodeAttrs[ n ] = a;
                    cerr << "n:" << n << " a:" << a << endl;
                }
            }
        }
    

        fh_domdoc doc = Factory::StreamToDOM( Factory::fcin() );

        PerformUniqueNodeAttrs( doc, doc->getDocumentElement() );
        
        fh_stringstream oss = tostream( doc, true );
        std::copy( std::istreambuf_iterator<char>(oss),
                   std::istreambuf_iterator<char>(),
                   std::ostreambuf_iterator<char>(cout));

    }
    catch( NoSuchContextClass& e )
    {
        cerr << "e:" << e.what() << endl;
        exit(1);
    }
    catch( exception& e )
    {
        cerr << "cought:" << e.what() << endl;
        exit(1);
    }

    return(0);
}
