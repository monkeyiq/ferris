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

    $Id: SpiritFFilter.cpp,v 1.2 2010/09/24 21:31:00 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

/*
 * This is a part start on an implementation using Boost::Spirit
 * to parse ffilter strings. I stopped on 19-sep-2005 because:
 * http://boost.org/libs/spirit/example/fundamental/regular_expression.cpp
 * /usr/include/boost/spirit/utility/impl/regex.ipp:65: error: wrong number of template arguments (1, should be 2)
 * /usr/include/boost/regex/v4/basic_regex.hpp:558: error: provided for template<class charT, class traits> class boost::reg_expression
 *
 * So basically I couldn't use spirits regex_p() to parse low level constructs
 * and the PCCTS ffitler implementation works, PCCTS isn't changing etc.
 *
 * I'll have another crack at this when boost > 1.33 comes out.
 */

// #include <Ferris/Ferris.hh>
// #include <Ferris/Resolver_private.hh>

// #define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 3

// #include <boost/spirit.hpp>
// #include <boost/spirit/utility/regex.hpp>
// using namespace boost::spirit;

// #include <boost/function.hpp>
// #include <boost/lambda/lambda.hpp>
// using namespace boost::lambda;

// using namespace std;

// namespace Ferris
// {

//     /**
//      * Root context for ffilter syntax strings. ffilter://
//      */
//     class FERRISEXP_DLLLOCAL SpiritFFilterRootContext
//         :
//         public StateLessEAHolder< SpiritFFilterRootContext, FakeInternalContext >
//     {
//         typedef SpiritFFilterRootContext _Self;
//         typedef StateLessEAHolder< SpiritFFilterRootContext, FakeInternalContext > _Base;

//         void parse_ffilter( string& data );
        
//     public:

//         SpiritFFilterRootContext( string& data );
//         virtual ~SpiritFFilterRootContext();

//         void createStateLessAttributes( bool force = false );
//     };

//     SpiritFFilterRootContext::SpiritFFilterRootContext( string& data )
//         :
//         _Base( 0, "/" )
//     {
//         createStateLessAttributes();
//         parse_ffilter( data );
//     }
    
//     SpiritFFilterRootContext::~SpiritFFilterRootContext()
//     {
//     }

//     void
//     SpiritFFilterRootContext::createStateLessAttributes( bool force )
//     {
//         if( force || isStateLessEAVirgin() )
//         {
//             _Base::createStateLessAttributes( true );
//             supplementStateLessAttributes( true );
//         }
//     }

//     void
//     SpiritFFilterRootContext::parse_ffilter( string& data )
//     {
//         typedef scanner_list<scanner<>, phrase_scanner_t> scanners;
//         typedef rule< scanners > R;

//         R comparison = str_p("=~")
//             | str_p("<=") | str_p(">=") | str_p("==")
//             | str_p("<?=") | str_p(">?=") | str_p("=?=");

//         R eakey = regex_p("[ a-zA-Z0-9\\-\\_\\*\\$\\^\\]\\[\\\\\\/\\~\\:]+");
//         R value = regex_p("[^)]+)");
//         R eamatch = eakey comparison eavalue;
//         R eaterm = str_p("(")
//             >> ( eamatch
//                  | ( "!" >> eaterm >> ")" )
//                  | ( "&" >> eaterm >> ")" )
//                  | ( "|" >> eaterm >> ")" )
//                 );
//         R ffilter_p = eaterm;
        
//         parse_info<> info = parse(
//             data.c_str(),
//             ffilter_p,
//             space_p );

//         if (info.full)
//         {
//             return;
//         }
//         else
//         {
//             fh_stringstream ss;
//             ss << "Parsing ffilter string failed" << nl
//                << "input:" << data << nl
//                << "stopped at: \": " << info.stop << "\"" << nl
//                << "char offset:" << ( info.stop - data.c_str() ) << nl;
//             LG_CTX_W << tostr(ss) << endl;
//             cerr << tostr(ss) << endl;
//             Throw_ParseError( tostr(ss), 0 );
//         }
            
        
//     }
    
    
//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/

//     class FERRISEXP_DLLLOCAL SpiritFFilterRootContext_RootContextDropper
//         :
//         public RootContextDropper
//     {
//     public:
//         SpiritFFilterRootContext_RootContextDropper()
//             {
//                 ImplementationDetail::appendToStaticLinkedRootContextNames("ffilter");
//                 RootContextFactory::Register( "ffilter", this );
//             }

//         fh_context Brew( RootContextFactory* rf )
//             throw( RootContextCreationFailed )
//             {
//                 fh_context ret = 0;
//                 string data;
                
//                 if( rf->getInfo( "StaticString" ).length() )
//                 {
//                     data = rf->getInfo( "StaticString" );
//                 }
                    
//                 ret = new SpiritFFilterRootContext( data );
//                 return ret;
//             }
//     };
//     static SpiritFFilterRootContext_RootContextDropper ___SpiritFFilterRootContext_static_init;

//     /********************************************************************************/
//     /********************************************************************************/
//     /********************************************************************************/

// };

