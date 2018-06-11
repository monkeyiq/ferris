/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2011 Ben Martin

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

    $Id: libffilter.cpp,v 1.3 2010/09/24 21:31:43 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
//#define BOOST_SPIRIT_DUMP_PARSETREE_AS_XML



#include <Ferris/Ferris.hh>
#include <Resolver_private.hh>

#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 3

#include <boost/spirit/home/classic/core.hpp>
#include <boost/spirit/home/classic/tree/parse_tree.hpp>
#include <boost/spirit/home/classic/tree/ast.hpp>
#include <boost/spirit/home/classic/utility/regex.hpp>
using namespace boost::spirit;
#ifdef BOOST_SPIRIT_DUMP_PARSETREE_AS_XML
#include <boost/spirit/tree/tree_to_xml.hpp>
#include <map>
#endif

#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
using namespace boost::lambda;

using namespace std;

#define DEBUG LG_SPIRITCONTEXT_D

using namespace boost::spirit::classic;

namespace Ferris
{
//    typedef scanner_list<scanner<>, phrase_scanner_t> scanners;
//    typedef rule< scanners > R;
    typedef char const*         iterator_t;
    typedef tree_match<iterator_t> parse_tree_match_t;
    typedef parse_tree_match_t::const_tree_iterator iter_t;
    
    typedef pt_match_policy<iterator_t> match_policy_t;
    typedef scanner_policies<
          iteration_policy,
          match_policy_t,
          action_policy> scanner_policy_t;
    typedef scanner<iterator_t, scanner_policy_t> scanner_t;
    typedef rule<scanner_t> rule_t;
    typedef rule_t R;

    void decend(iter_t const& i, string pfx = "" )
    {
        DEBUG << "decend " << pfx << " i->value = " <<
            string(i->value.begin(), i->value.end()) <<
            " i->children.size() = " << i->children.size() << endl;

        for( iter_t iter = i->children.begin(); iter != i->children.end(); ++iter )
        {
            decend( iter, pfx + "  " );
        }
    }

    void swap( iter_t&aiter, iter_t&biter )
    {
        parse_tree_match_t::node_t& a = const_cast<parse_tree_match_t::node_t&>(*aiter);
        parse_tree_match_t::node_t& b = const_cast<parse_tree_match_t::node_t&>(*biter);
        a.swap(b);

//        a.value.swap( b.value );
        
//      std::swap( *aiter, *biter );
//      a.swap( b );
    }

    void swapv( iter_t&aiter, iter_t&biter )
    {
        parse_tree_match_t::node_t& a = const_cast<parse_tree_match_t::node_t&>(*aiter);
        parse_tree_match_t::node_t& b = const_cast<parse_tree_match_t::node_t&>(*biter);
        a.value.swap( b.value );
    }


    void adjustRaiseGeneric(iter_t i,
                            std::map<parser_id, std::string>& rule_names,
                            bool (*func)(iter_t i, std::map<parser_id, std::string>& )
        )
    {
        if( !i->children.empty() )
        {
            iter_t iter = i->children.begin();
            if( iter->children.size() == 1 )
                iter = iter->children.begin();
            DEBUG << "first child:" << rule_names[iter->value.id()] << endl;
            if( func( iter, rule_names ) )
            {
                DEBUG << "raising operator &|!" << endl;
                swapv( i, iter );
            }
        }
        for( iter_t iter = i->children.begin(); iter != i->children.end(); ++iter )
        {
            adjustRaiseGeneric( iter, rule_names, func );
        }
    }
    

    class SpiritContext;
    FERRIS_SMARTPTR( SpiritContext, fh_chcontext );

    class SpiritContext
        :
        public FakeInternalContext
    {
        typedef FakeInternalContext _Base;
        typedef SpiritContext       _Self;

        typedef std::list< fh_context > InOrderInsertList_t;
        InOrderInsertList_t InOrderInsertList;
        
    protected:

        FakeInternalContext* priv_CreateContext( Context* parent, std::string rdn )
        {
            SpiritContext* ret = new SpiritContext( parent, rdn );
            ret->setContext( parent, rdn );
            return ret;
        }
        
    public:

        SpiritContext( Context* parent, const std::string& rdn )
            : FakeInternalContext( parent, rdn )
        {
            createStateLessAttributes();
            supplementStateLessAttributes();

            addAttribute( "token", rdn );
            addAttribute( "in-order-insert-list",
                          this, &_Self::getInOrderInsertListStream );
        }
        virtual ~SpiritContext()
        {
            cerr << "~SpiritContext()" << endl;
        }
        

        static fh_chcontext Create( fh_chcontext parent, const std::string& rdn )
            {
                fh_chcontext ret(new SpiritContext( GetImpl(parent), rdn ));
                ret->setContext( GetImpl(parent), parent->monsterName(rdn) );

                parent->Insert( GetImpl(ret) );
//                ret->addAttribute( (std::string)"token",
//                                   SL_getDirNameStream );

                return ret;
            }
        
        void makeTree( fh_chcontext parent, iter_t i, std::map<parser_id, std::string>& rule_names )
        {
            string rdn = string(i->value.begin(), i->value.end());
            DEBUG << "makeTree(s) rdn:" << rdn << " parent:" << parent->getURL() << endl;

            fh_chcontext cc = parent;
            if( !rdn.empty() )
            {
                cc = SpiritContext::Create( parent, rdn );
            }
            
            for( iter_t iter = i->children.begin(); iter != i->children.end(); ++iter )
            {
                makeTree( cc, iter, rule_names );
            }
            DEBUG << "makeTree(e) rdn:" << rdn << " parent:" << parent->getURL() << endl;
            
        }
        

        virtual fh_context Insert( Context* ctx, bool created = false, bool emit = true )
            throw( SubContextAlreadyInUse )
            {
                DEBUG << "Insert() Path:" << getDirPath()
                      << " ctx path:" << ctx->getDirPath()
                      << endl;
                InOrderInsertList.push_back( ctx );
                fh_context ret = _Base::Insert( ctx );
                return ret;
            }
        fh_istream getInOrderInsertListStream( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;
                bool virgin = true;

                LG_PCCTS_D << "get_InOrderInsertList_AsString() " << endl;
                for( InOrderInsertList_t::const_iterator iter = InOrderInsertList.begin();
                     iter != InOrderInsertList.end();
                     ++iter )
                {
                    virgin = false;
                    ss << (*iter)->getDirName() << endl;
                    LG_PCCTS_D << "get_InOrderInsertList_AsString() adding:"
                               << (*iter)->getDirName()
                               << endl;
                }

                if(virgin)
                {
                    ss << "";
                }
            
                return ss;
            }
    
        
        
    };
    
        
    Context* BrewSpirit( R topLevel,
                         RootContextFactory* rf,
                         std::map<parser_id, std::string>& rule_names,
                         void (*adjust)(iter_t i, std::map<parser_id, std::string>& )
        )
    {
        std::string data = rf->getInfo( "StaticString" );

        // parse_info<> info = parse( data.c_str(), topLevel, space_p );
        tree_parse_info<> info = ast_parse( data.c_str(), topLevel, space_p );

//        ast_scanner_t scanner( data.c_str(), data.c_str()+data.length() );
//        tree_parse_info<> info = ast_parse( scanner, topLevel, space_p );
        
        if (info.full)
        {
            DEBUG << "parsing was ok." << endl;

#if defined(BOOST_SPIRIT_DUMP_PARSETREE_AS_XML)
            tree_to_xml(cout, info.trees, data.c_str(), rule_names);
#endif
            DEBUG << "-----------------------------------------" << endl;
            adjust( info.trees.begin(), rule_names );
            decend( info.trees.begin() );

            SpiritContext* ret = new SpiritContext( 0, "" );
            fh_chcontext cc = ret;
            cc->makeTree( cc, info.trees.begin(), rule_names );
            cc->dumpTree();
            return ret;
        }
        else
        {
            fh_stringstream ss;
            ss << "Parsing spirit context failed" << endl
            << "input:" << data << nl
            << "stopped at: \": " << info.stop << "\"" << nl
            << "char offset:" << ( info.stop - data.c_str() ) << endl;
            cerr << tostr(ss) << endl;
        }

        return 0;
    }
}
