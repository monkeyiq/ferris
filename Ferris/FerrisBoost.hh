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

    $Id: FerrisBoost.hh,v 1.11 2010/09/24 21:30:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_BOOST_H_
#define _ALREADY_INCLUDED_FERRIS_BOOST_H_

#include <boost/regex.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/lambda/lambda.hpp>
using namespace boost::lambda;

#include <Ferris/TypeDecl.hh>
#include <string>
#include <list>

namespace Ferris
{
#ifdef GCC_HASCLASSVISIBILITY
#pragma GCC visibility push(default)
#endif

    typedef std::list< boost::regex > regexlist_t;

    /**
     * compile each item in 'sl' into a regex and place it into ret.
     * return ret.
     */
    regexlist_t& stringlist_to_regexlist( regexlist_t& ret, const stringlist_t& sl );

    /**
     * anything in 'sl' which matches any regex in 'rel' is removed from 'sl'
     */
    void erase_any_matches( regexlist_t& rel, stringlist_t& sl );
    void erase_any_matches( const boost::regex& r, stringlist_t& sl );


    /**
     * Convert a list of regexs to a single composite regex.
     * eg, abstractly stringlist is <a,b,c> elements, regex is (a|b|c).
     *
     * Common prefixes are shifted out, for example:
     * file://foo
     * file://bar
     * gives file://(foo|bar)
     */
    std::string MakeCompositeRegexString( const stringlist_t& sl );

//     typedef Loki::SmartPtr< RootContextFactory, 
//                             Loki::RefLinked, 
//                             Loki::AllowConversion, 
//                             FerrisLoki::FerrisExSmartPointerChecker, 
//                             Loki::DefaultSPStorage >  fh_regex;

    
    /**
     * Create a boost::regex using common optimization options.
     */
    boost::regex toregex( const std::string& s, boost::regex::flag_type rflags = boost::regex::optimize );
    boost::regex toregex( const stringlist_t& sl, boost::regex::flag_type rflags = boost::regex::optimize );
    fh_rex     toregexh( const std::string& s,   boost::regex::flag_type rflags = boost::regex::optimize );
    fh_rex     toregexh( const stringlist_t& sl, boost::regex::flag_type rflags = boost::regex::optimize );

    /**
     * Automatically add an icase flag. sorthand for toregex( foo, boost::regex::icase ).
     * Note that if rflags is given then icase is implicitly added aswell
     */
    boost::regex toregexi( const std::string& s, boost::regex::flag_type rflags = boost::regex::icase );
    boost::regex toregexi( const stringlist_t& sl, boost::regex::flag_type rflags = boost::regex::icase );
    fh_rex     toregexhi( const std::string& s,   boost::regex::flag_type rflags = boost::regex::icase );
    fh_rex     toregexhi( const stringlist_t& sl, boost::regex::flag_type rflags = boost::regex::icase );

    /**
     * regex_match() has to match the whole string, regex_search() can be partial.
     */
    inline bool regex_match(const std::string& s, 
                            boost::smatch& m,
                            const fh_rex& e, 
                            boost::match_flag_type flags = boost::match_default )
    {
        return regex_match( s, m, *e, flags);
    }
    inline bool regex_match(const std::string& s, 
                            const fh_rex& e, 
                            boost::match_flag_type flags = boost::match_default )
    {
        return regex_match( s, *e, flags);
    }


    inline bool regex_match(const std::string& s, 
                            boost::smatch& m,
                            const RegexCollection& e, 
                            boost::match_flag_type flags = boost::match_default )
    {
        return regex_match( s, m, e.getRegex(), flags);
    }
    inline bool regex_match(const std::string& s, 
                            const RegexCollection& e, 
                            boost::match_flag_type flags = boost::match_default )
    {
        return regex_match( s, e.getRegex(), flags);
    }

    /**
     * The regex_match_single() family allow you to pass a string and a
     * regex to pluck out a single match with a (capture) expression.
     * the capture is returned or "" if the regex didn't match exactly
     * one capture.
     */
    std::string regex_match_single( const std::string& s, 
                               const fh_rex& e, 
                               boost::match_flag_type flags = boost::match_default );
    std::string regex_match_single( const std::string& s, 
                               const std::string& rex,
                               boost::match_flag_type flags = boost::match_default );
    
    
    
    
    inline bool regex_search( const std::string& s, 
                              boost::smatch& m,
                              const fh_rex& e, 
                              boost::match_flag_type flags = boost::match_default )
    {
        return regex_search( s, m, *e, flags );
    }
    inline bool regex_search( const std::string& s, 
                              const fh_rex& e, 
                              boost::match_flag_type flags = boost::match_default )
    {
        return regex_search( s, *e, flags );
    }
    inline bool regex_search( const std::string& s, 
                              boost::smatch& m,
                              const RegexCollection& e, 
                              boost::match_flag_type flags = boost::match_default )
    {
        return regex_search( s, m, e.getRegex(), flags );
    }
    inline bool regex_search( const std::string& s, 
                              const RegexCollection& e, 
                              boost::match_flag_type flags = boost::match_default )
    {
        return regex_search( s, e.getRegex(), flags );
    }

    template <class traits, class charT>
    std::basic_string<charT> replaceg( const std::basic_string<charT>& s,
                                       const boost::basic_regex<charT, traits>& e,
                                       const std::basic_string<charT>& fmt,
                                       boost::match_flag_type flags = boost::match_default | boost::format_all )
    {
        return boost::regex_replace( s, e, fmt, flags | boost::match_default | boost::format_all );
    }
    template <class traits, class charT>
    std::basic_string<charT> replaceg( const std::basic_string<charT>& s,
                                       const boost::basic_regex<charT, traits>& e,
                                       const char* fmt,
                                       boost::match_flag_type flags = boost::match_default | boost::format_all )
    {
        return boost::regex_replace( s, e, fmt, flags | boost::match_default | boost::format_all );
    }
    
    template <class charT>
    std::basic_string<charT> replaceg( const std::basic_string<charT>& s,
                                       fh_rex e,
                                       const std::basic_string<charT>& fmt,
                                       boost::match_flag_type flags = boost::match_default | boost::format_all )
    {
        return boost::regex_replace( s, *e, fmt, flags | boost::match_default | boost::format_all );
    }
    template <class charT>
    std::basic_string<charT> replaceg( const std::basic_string<charT>& s,
                                       fh_rex e,
                                       const char* fmt,
                                       boost::match_flag_type flags = boost::match_default | boost::format_all )
    {
        return boost::regex_replace( s, *e, fmt, flags | boost::match_default | boost::format_all );
    }

    /**
     * Convenience function for code that wants a staticly compiled regex
     * but does not want to worry about singletons or the like.
     * rex is compiled to a regex object and cached for future calls.
     */
    std::string replaceg( const std::string& s,
                          const std::string& rex,
                          const std::string& fmt = "\\1",
                          boost::match_flag_type flags = boost::match_default | boost::format_all );
    
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    // 
    // Allow quick and easy single time forward sweeps of begin() to end()
    //
    //     typedef list< string > stringlist_t;
    //     stringlist_t sl;
    //     sl.push_back("foo");
    //     sl.push_back("bar");
    //     sl.push_back("bzt");
    //     for( rs<stringlist_t> r( sl ); r; ++r )
    //        {
    //           cerr << "r:" << *r << endl;
    //        }



    template <class Col>
    class range_sweep
        : public boost::iterator_adaptor< range_sweep<Col>, typename Col::iterator >
    {
        typedef boost::iterator_adaptor< range_sweep<Col>, typename Col::iterator > super_t;
        typedef typename Col::iterator Iterator;
      
        friend class ::boost::iterator_core_access;

        Iterator m_e;
      
    public:
//      range_sweep() {}

        explicit range_sweep( Iterator b, Iterator e )
            : super_t(b), m_e(e) {}
        explicit range_sweep( Col& c )
            : super_t( c.begin() ), m_e( c.end() ) {}

//       template<class OtherIterator>
//       range_sweep(
//           range_sweep<OtherIterator> const& r
//           , typename enable_if_convertible<OtherIterator, Iterator>::type* = 0
//           )
//           : super_t(r.base())
//       {}

        operator bool()
            {
                return this->base_reference()!=m_e;
            }
      
    private:
        typename super_t::reference dereference() const
            {
                return *(this->base_reference());
            }

        void increment() { ++this->base_reference(); }
        void decrement() { --this->base_reference(); }

        void advance(typename super_t::difference_type n)
            {
                this->base_reference() += n;
            }
    };

    template <class Col>
    range_sweep<Col> make_range_sweep( Col& c )
    {
        return range_sweep<Col>( c.begin(), c.end() );
    }

    template <class Col>
    class rs : public range_sweep<Col>
    {
    public:
        explicit rs( typename Col::iterator b, typename Col::iterator e )
            :
            range_sweep<Col>(b,e) {}
        explicit rs( Col& c )
            : range_sweep<Col>( c ) {}
    };
//      template <class Col>
//      rs<Col> mrs( Col& c )
//      {
//          return rs<Col>( c.begin(), c.end() );
//      }
    

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

#ifdef GCC_HASCLASSVISIBILITY
#pragma GCC visibility pop
#endif
    
};
#endif
