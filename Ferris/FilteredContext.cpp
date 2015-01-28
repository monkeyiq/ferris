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

    $Id: FilteredContext.cpp,v 1.19 2011/05/04 21:30:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FilteredContext.hh>
#include <FilteredContext_private.hh>
#include <unistd.h>

using namespace std;

#include <string>
#include <Singleton.h>
#include <Factory.h>
#include <Functor.h>
#include <SM.hh>
#include <Resolver_private.hh>
#include <Ferris_private.hh>
#include <SignalStreams.hh>
#include <General.hh>
#include <FerrisBoost.hh>

namespace Ferris
{
    /**
     * Get the infered type of the data in the val param.
     * The return value will be one of the strings from
     * xsd/attributes/attributedomain/possiblesort
     */
    std::string guessComparisonOperatorFromData( const std::string& val )
    {
        std::string ret = "binary";

        typedef list< pair< fh_regex, string > > Inferences_t;
        Inferences_t Inferences;

        Inferences.push_back( make_pair( new Regex("^[0-9]+$"),                      "int" ));
        Inferences.push_back( make_pair( new Regex("^[0-9]+\\.[0-9]+$"),             "double" ));
        Inferences.push_back( make_pair( new Regex("^[a-zA-Z0-9\\.\\*\\-\\_\\ ]+$"), "string" ));
        Inferences.push_back( make_pair( new Regex("^[a-z0-9\\.\\*\\-\\_\\ ]+$"),    "cis" ));
        
        for( Inferences_t::iterator ii = Inferences.begin(); ii != Inferences.end(); ++ii )
        {
            if( ii->first->operator()( val ) )
                return ii->second;
        }
        return ret;
    }
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    inline bool getEAStringOrFalse( const fh_context& c,
                                    const std::string eaname,
                                    std::string& val )
    {
        try
        {
            if( !isBound(c) )
                return false;
            
            fh_attribute a = c->getAttribute( eaname );
            fh_istream ss  = a->getIStream();
            return getline( ss, val );
        }
        catch(...)
        {
        }
        return false;
    }

    inline bool getEALongOrFalse( const fh_context& c,
                                  const std::string eaname,
                                  long& val )
    {
        try
        {
            if( !isBound(c) )
                return false;
            
            fh_attribute a = c->getAttribute( eaname );
            fh_istream ss  = a->getIStream();
            return (ss >> val);
        }
        catch(...)
        {
        }
        return false;
    }


    inline bool getEADoubleOrFalse( const fh_context& c,
                                    const std::string eaname,
                                    double& val )
    {
        try
        {
            if( !isBound(c) )
                return false;
            
            fh_attribute a = c->getAttribute( eaname );
            fh_istream ss  = a->getIStream();
            return (ss >> val);
        }
        catch(...)
        {
        }
        return false;
    }

    template <class T>
    inline bool getEAOrFalse( const fh_context& c,
                              const std::string eaname,
                              T& val )
    {
        try
        {
            if( !isBound(c) )
                return false;
            
            fh_attribute a = c->getAttribute( eaname );
            fh_istream ss  = a->getIStream();
            return (ss >> val);
        }
        catch(...)
        {
        }
        return false;
    }

    /**
     * Perform some caching of what sorting order to use for the given EA.
     * Note that the extra value "time" is added to those returned by getSchemaDefaultSort().
     * this is done so that conversions from relative time deltas as strings can be converted
     * into an absolute time_t value for comparison.
     *
     * Also a extra value of "size" can be returned to allow automatic conversion
     * of strings such as 100k or 5.5m to their integer version.
     *
     * This class will return the same sort order for all contexts with
     * a common parent, once the parent changes the sort order can change too.
     *
     */
    struct FERRISEXP_DLLLOCAL schemaCache
    {
        string cname;
        fh_context parent;
        bool EAIsStatless;
        bool HaveTestedEAIsStatless;
        
        schemaCache()
            :
            cname(""), parent(0), EAIsStatless(false), HaveTestedEAIsStatless(false)
            {
            }
        
        inline const std::string& getSchema( const std::string& eaname, const fh_context& c )
            {
                bool updateCachedName = cname.empty() || !parent;

                //
                // Different parent taints the cache.
                //
                if( !updateCachedName && c->isParentBound() && parent != c->getParent() )
                {
                    updateCachedName = true;
                    parent = c->getParent();
                }

                //
                // For contexts which can change the schema for the same attribute
                // in the same directory we have to not cache the schema unless we
                // are looking at an attribute which is stateless and thus has a
                // fixed schema still.
                //
                if( !updateCachedName
                    && !EAIsStatless
                    && parent
                    && !parent->getSubContextAttributesWithSameNameHaveSameSchema() )
                {
                    if( !HaveTestedEAIsStatless )
                    {
                        HaveTestedEAIsStatless = true;
                        EAIsStatless = c->isStatelessAttributeBound( eaname );
                    }

                    updateCachedName = EAIsStatless;
                }

                //
                // if the cache is tainted then update the sort specifier
                //
                if( updateCachedName )
                {
                    fh_context sc    = c->getSchema( eaname );
                    stringlist_t sol = getSchemaDefaultSortList( sc );
                    cname = sol.front();

                    XSDBasic_t sct = getSchemaType( c, eaname, XSD_UNKNOWN );
                    
//                    cerr << "getSchema(update) sct:" << sct << endl;
                    if( FXD_UNIXEPOCH_T == sct )
                        cname = "time";
                    else if( FXD_FILESIZE == sct )
                        cname = "size";
                }
                return cname;
            }

        inline const std::string& operator()( const std::string& eaname, const fh_context& c )
            {
                return getSchema( eaname, c );
            }
    };
    

    struct FERRISEXP_DLLLOCAL matcher_typesafe_base
    {
        string value;
        std::string eaname;
        mutable schemaCache m_schemaCache;
        
        matcher_typesafe_base( const std::string& eaname, const std::string& value )
            :
            eaname( eaname ),
            value( value )
            {
            }

        typedef Loki::AssocVector< string, fh_matcher > FunctionTable_t;
        mutable FunctionTable_t FunctionTable;
        inline FunctionTable_t& getFunctionTable() const
            {
                return FunctionTable;
            }

        inline bool operator()( const fh_context& c ) const
            {
                const std::string& cname = m_schemaCache( eaname, c );
                
                FunctionTable_t::iterator fi = getFunctionTable().find( cname );
                if( fi != getFunctionTable().end() )
                {
                    fi->second( c );
                }
            }
    };

    /********************/
    /********************/
    /********************/
    
    struct FERRISEXP_DLLLOCAL matcher_presence
    {
        std::string eaname;
        matcher_presence( const std::string& eaname ) : eaname( eaname ) {}

        inline bool operator()( const fh_context& c ) const
            {
                return( c->isAttributeBound(eaname) );
            }
    };

    /********************/
    /********************/
    /********************/
    
    struct FERRISEXP_DLLLOCAL matcher_MakeHasOneOrMoreBytesMatcher
    {
        std::string eaname;
        bool m_createIfNotThere;
        
        matcher_MakeHasOneOrMoreBytesMatcher( const std::string& eaname,
                                              bool createIfNotThere = true )
            :
            eaname( eaname ),
            m_createIfNotThere( createIfNotThere )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                if( !isBound(c) )
                    return false;
                
                if( !c->isAttributeBound(eaname, m_createIfNotThere ) )
                    return false;

                try
                {
                    fh_attribute a = c->getAttribute( eaname );
                    fh_istream iss = a->getIStream();
                    char ch;
                    return iss >> ch;
                }
                catch( exception& e )
                {
                    return false;
                }
            }
    };

    /********************/
    /********************/
    /********************/

    struct FERRISEXP_DLLLOCAL matcher_EAValueGreaterEqMTime
    {
        std::string eaname;
        matcher_EAValueGreaterEqMTime( const std::string& eaname ) : eaname( eaname ) {}

        inline bool operator()( const fh_context& c ) const
            {
                if( !isBound(c) )
                    return false;
                
                time_t eaname_tt = toType< time_t >( getStrAttr( c, eaname, "0" ));
                time_t  mtime_tt = toType< time_t >( getStrAttr( c, "mtime", "1" ));

//                 cerr << "matcher_EAValueGreaterEqMTime() c:" << c->getURL() << endl
//                      << "   eaname:" << eaname
//                      << " eaname_tt:" << eaname_tt
//                      << " mtime_tt:" << mtime_tt
//                      << " ret:" << ( eaname_tt >= mtime_tt )
//                      << endl;
                
                return( eaname_tt >= mtime_tt );
            }
    };

    /********************/
    /********************/
    /********************/
    
    struct FERRISEXP_DLLLOCAL matcher_equal
    {
        std::string eaname;
        std::string value;

        matcher_equal( const std::string& eaname,
                       const std::string& value )
            :
            eaname( eaname ),
            value( value )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                string s;
                return getEAStringOrFalse( c, eaname, s )
                    && s == value;
            }
    };

    struct FERRISEXP_DLLLOCAL matcher_typesafe_equal
        :
        public matcher_typesafe_base
    {
        
        matcher_typesafe_equal( const std::string& eaname, const std::string& value )
            :
            matcher_typesafe_base( eaname, value )
            {
                FunctionTable["string"] = Factory::MakeEqualBinaryMatcher( eaname, value );
                FunctionTable["cis"]    = Factory::MakeEqualCISMatcher( eaname, value );
                FunctionTable["double"] = Factory::MakeEqualDoubleMatcher( eaname, toType<double>(value));
                FunctionTable["float"]  = Factory::MakeEqualDoubleMatcher( eaname, toType<double>(value));
                FunctionTable["int"]    = Factory::MakeEqualIntegerMatcher( eaname, toType<long>(value));
                FunctionTable["binary"] = Factory::MakeEqualBinaryMatcher( eaname, value );

                time_t tt = Time::ParseRelativeTimeString( value );
                FunctionTable["time"] = Factory::MakeEqualIntegerMatcher( eaname, tt );

                guint64 sz = Util::convertByteString( value );
                FunctionTable["size"] = Factory::MakeEqualIntegerMatcher( eaname, sz );
            }
    };
    

    /********************/
    /********************/
    /********************/

    struct FERRISEXP_DLLLOCAL matcher_endswith
    {
        std::string eaname;
        std::string value;

        matcher_endswith( const std::string& eaname,
                          const std::string& value )
            :
            eaname( eaname ),
            value( value )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                string s;
                return getEAStringOrFalse( c, eaname, s )
                    && ends_with( s, value );
            }
    };
    struct FERRISEXP_DLLLOCAL matcher_endswith_cis
    {
        std::string eaname;
        std::string value;

        matcher_endswith_cis( const std::string& eaname,
                              const std::string& value )
            :
            eaname( eaname ),
            value( toupperstring(value) )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                string s;
                return getEAStringOrFalse( c, eaname, s )
                    && ends_with( toupperstring(s), value );
            }
    };


    /********************/
    /********************/
    /********************/

    struct FERRISEXP_DLLLOCAL matcher_startswith
    {
        std::string eaname;
        std::string value;

        matcher_startswith( const std::string& eaname,
                          const std::string& value )
            :
            eaname( eaname ),
            value( value )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                string s;
                return getEAStringOrFalse( c, eaname, s )
                    && starts_with( s, value );
            }
    };
    struct FERRISEXP_DLLLOCAL matcher_startswith_cis
    {
        std::string eaname;
        std::string value;

        matcher_startswith_cis( const std::string& eaname,
                              const std::string& value )
            :
            eaname( eaname ),
            value( toupperstring(value) )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                string s;
                return getEAStringOrFalse( c, eaname, s )
                    && starts_with( toupperstring(s), value );
            }
    };

    
    /********************/
    /********************/
    /********************/

    struct FERRISEXP_DLLLOCAL matcher_regex
    {
        mutable fh_rex r;
        std::string eaname;

        matcher_regex( const std::string& eaname,
                       const std::string& value )
            :
            eaname( eaname ),
            r( toregexhi( value ) )
            {
                static boost::regex hasUppers = toregex( "[A-Z]" );
                if( regex_search( value, hasUppers ))
                {
                    r = toregexh( value );
                }
            }

        inline bool operator()( const fh_context& c ) const
            {
                string s;
                return getEAStringOrFalse( c, eaname, s )
                    && regex_search( s, r );
//                    && r->operator()( s );
            }
    };
    

    /**********/
    /**********/
    /**********/
    
//     struct matcher_greq
//     {
//         long value;
//         std::string eaname;

//         matcher_greq( const std::string& eaname, long value )
//             :
//             eaname( eaname ),
//             value( value )
//             {}

//         inline bool operator()( const fh_context& c ) const
//             {
//                 long v = 0;
//                 return getEALongOrFalse( c, eaname, v )
//                     && v >= value;
//             }
//     };

    struct FERRISEXP_DLLLOCAL matcher_typesafe_greq
        :
        public matcher_typesafe_base
    {
        
        matcher_typesafe_greq( const std::string& eaname, const std::string& value )
            :
            matcher_typesafe_base( eaname, value )
            {
                FunctionTable["string"] = Factory::MakeGrEqBinaryMatcher( eaname, value );
                FunctionTable["cis"]    = Factory::MakeGrEqCISMatcher( eaname, value );
                FunctionTable["double"] = Factory::MakeGrEqDoubleMatcher( eaname, toType<double>(value));
                FunctionTable["float"]  = Factory::MakeGrEqDoubleMatcher( eaname, toType<double>(value));
                FunctionTable["int"]    = Factory::MakeGrEqMatcher( eaname, toType<long>(value));
                FunctionTable["binary"] = Factory::MakeGrEqBinaryMatcher( eaname, value );

                time_t tt = Time::ParseRelativeTimeString( value );
                FunctionTable["time"] = Factory::MakeGrEqMatcher( eaname, tt );

                guint64 sz = Util::convertByteString( value );
                FunctionTable["size"] = Factory::MakeGrEqMatcher( eaname, sz );
                
            }
    };
    
    /**********/
    /**********/
    /**********/
    
    template< class T, class Compare >
    struct matcher_compare_numeric
    {
        T value;
        std::string eaname;
            
        matcher_compare_numeric( const std::string& eaname, T value )
            :
            eaname( eaname ),
            value( value )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                T v = 0;
                return getEAOrFalse<T>( c, eaname, v ) && Compare()( v, value );
            }
    };

    template< class Compare >
    struct matcher_compare_case_sensitive_string
    {
        const std::string value;
        std::string eaname;
            
        matcher_compare_case_sensitive_string( const std::string& eaname, const std::string& value )
            :
            eaname( eaname ),
            value( value )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                std::string v = "";
                return getEAStringOrFalse( c, eaname, v ) && Compare()( v, value );
            }
    };
    
    template< class Compare >
    struct matcher_compare_case_insensitive_string
    {
        const std::string value;
        std::string eaname;
            
        matcher_compare_case_insensitive_string( const std::string& eaname, const std::string& value )
            :
            eaname( eaname ),
            value( tolowerstr()( value ) )
            {}

        inline bool operator()( const fh_context& c ) const
            {
                static tolowerstr tls;
                std::string v = "";
                return getEAStringOrFalse( c, eaname, v ) && Compare()( tls( v ), value );
            }
    };

    struct FERRISEXP_DLLLOCAL matcher_typesafe_lteq
        :
        public matcher_typesafe_base
    {
        matcher_typesafe_lteq( const std::string& eaname, const std::string& value )
            :
            matcher_typesafe_base( eaname, value )
            {
                FunctionTable["string"] = Factory::MakeLtEqBinaryMatcher( eaname, value );
                FunctionTable["cis"]    = Factory::MakeLtEqCISMatcher( eaname, value );
                FunctionTable["double"] = Factory::MakeLtEqDoubleMatcher( eaname, toType<double>(value));
                FunctionTable["float"]  = Factory::MakeLtEqDoubleMatcher( eaname, toType<double>(value));
                FunctionTable["int"]    = Factory::MakeLtEqMatcher( eaname, toType<long>(value));
                FunctionTable["binary"] = Factory::MakeLtEqBinaryMatcher( eaname, value );

                time_t tt = Time::ParseRelativeTimeString( value );
                FunctionTable["time"] = Factory::MakeLtEqMatcher( eaname, tt );

                guint64 sz = Util::convertByteString( value );
                FunctionTable["size"] = Factory::MakeLtEqMatcher( eaname, sz );
                
            }
    };

    /**********/
    /**********/
    /**********/

//     struct matcher_greq_double
//     {
//         double value;
//         std::string eaname;

//         matcher_greq_double( const std::string& eaname, double value )
//             :
//             eaname( eaname ),
//             value( value )
//             {}

//         inline bool operator()( const fh_context& c ) const
//             {
//                 double v = 0.0;
//                 return getEADoubleOrFalse( c, eaname, v )
//                     && v >= value;
//             }
//     };

//     struct matcher_lteq_double
//     {
//         double value;
//         std::string eaname;

//         matcher_lteq_double( const std::string& eaname, double value )
//             :
//             eaname( eaname ),
//             value( value )
//             {}

//         inline bool operator()( const fh_context& c ) const
//             {
//                 double v = 0.0;
//                 return getEADoubleOrFalse( c, eaname, v )
//                     && v <= value;
//             }
//     };

    struct FERRISEXP_DLLLOCAL matcher_not
    {
        fh_matcher fun;

        matcher_not( fh_matcher fun )
            :
            fun( fun )
            {}

        inline bool operator()( const fh_context& c )
            {
                return !fun(c);
            }
    };

    struct FERRISEXP_DLLLOCAL matcher_and
    {
        fh_matcher leftf;
        fh_matcher rightf;

        matcher_and( fh_matcher leftf, fh_matcher rightf )
            :
            leftf( leftf ),
            rightf( rightf )
            {}

        inline bool operator()( const fh_context& c )
            {
                return leftf( c ) && rightf( c );
            }
    };

    struct FERRISEXP_DLLLOCAL matcher_or
    {
        fh_matcher leftf;
        fh_matcher rightf;

        matcher_or( fh_matcher leftf, fh_matcher rightf )
            :
            leftf( leftf ),
            rightf( rightf )
            {}

        inline bool operator()( const fh_context& c )
            {
                return leftf( c ) || rightf( c );
            }
    };

    struct FERRISEXP_DLLLOCAL matcher_true
    {
        inline bool operator()( const fh_context& c ) const
            { return true; }
    };

    struct FERRISEXP_DLLLOCAL matcher_false
    {
        inline bool operator()( const fh_context& c ) const
            { return false; }
    };

    
    namespace Factory
    {
        template <class MatcherXType>
        fh_matcher ComposeXMatcher( const EndingList& el )
        {
            fh_matcher ret;
            Util::SingleShot virgin;
            
            for( EndingList::const_iterator iter = el.begin();
                 iter != el.end(); ++iter )
            {
                if( virgin() )
                {
                    ret = MatcherXType( iter->first, iter->second );
                }
                else
                {
                    fh_matcher f = MatcherXType( iter->first, iter->second );
                    ret = matcher_or( ret, f );
                }
            }
            return ret;
        }

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
        fh_matcher ComposeEqualsMatcher( const EndingList& el )
        {
            return ComposeXMatcher<matcher_equal>( el );
        }

        fh_matcher ComposeEndsWithMatcher( const EndingList& el, bool caseSensitive )
        {
            if( !caseSensitive )
                return ComposeXMatcher<matcher_endswith_cis>( el );
            return ComposeXMatcher<matcher_endswith>( el );
        }
    
        fh_matcher
        ComposeEndsWithMatcher( const char** beg, bool caseSensitive )
        {
            Factory::EndingList ret;
    
            const char** iter = beg;
            while( *iter )
            {
                string f = *iter;
                ++iter;
                string s = *iter;
                ++iter;

                ret.push_back( make_pair( f, s ) );
            }
            return ComposeEndsWithMatcher( ret, caseSensitive );
        }
    
        fh_matcher ComposeRegexMatcher( const EndingList& el )
        {
            return ComposeXMatcher<matcher_regex>( el );
        }
        
        fh_matcher MakeAlwaysTrueMatcher()
        {
            return matcher_true();
        }
        
        fh_matcher MakeAlwaysFalseMatcher()
        {
            return matcher_false();
        }

        fh_matcher MakeNotMatcher( const fh_matcher& m )
        {
            return matcher_not( m );
        }

        fh_matcher MakeOrMatcher( const fh_matcher& leftm, const fh_matcher& rightm )
        {
            return matcher_or( leftm, rightm );
        }

        fh_matcher MakeAndMatcher( const fh_matcher& leftm, const fh_matcher& rightm )
        {
            return matcher_and( leftm, rightm );
        }
        
        fh_matcher MakePresenceMatcher( const std::string& eaname )
        {
            return matcher_presence( eaname );
        }

        fh_matcher MakeHasOneOrMoreBytesMatcher( const std::string& eaname,
                                                 bool createIfNotThere )
        {
            return matcher_MakeHasOneOrMoreBytesMatcher( eaname, createIfNotThere );
        }
        
        fh_matcher MakeEAValueGreaterEqMTime( const std::string& eaname )
        {
            return matcher_EAValueGreaterEqMTime( eaname );
        }
        
        
        
        /********************/
        /********************/
        /********************/

        fh_matcher MakeEqualMatcher( const std::string& eaname, const std::string& value )
        {
            return MakeEqualBinaryMatcher( eaname, value );
        }
        fh_matcher MakeEqualIntegerMatcher( const std::string& eaname, long value )
        {
            return matcher_compare_numeric< long, equal_to< long > >( eaname, value );
        }
        fh_matcher MakeEqualDoubleMatcher( const std::string& eaname, double value )
        {
            return matcher_compare_numeric< double, equal_to< double > >( eaname, value );
        }
        fh_matcher MakeEqualBinaryMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_compare_case_sensitive_string< equal_to< string > >( eaname, value );
        }
        fh_matcher MakeEqualCISMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_compare_case_insensitive_string< equal_to< string > >( eaname, value );
        }
        fh_matcher MakeTypeSafeEqualMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_typesafe_equal( eaname, value );
        }
        
        
        /********************/
        /********************/
        /********************/
        
        fh_matcher MakeEndsWithMatcher( const std::string& eaname,
                                        const std::string& value )
        {
            return matcher_endswith( eaname, value );
        }
        fh_matcher MakeStartsWithMatcher( const std::string& eaname,
                                          const std::string& value )
        {
            return matcher_startswith( eaname, value );
        }
        
        

        fh_matcher MakeRegexMatcher( const std::string& eaname,
                                     const std::string& value )
        {
            return matcher_regex( eaname, value );
        }

        /********************/
        /********************/
        /********************/

        fh_matcher MakeGrEqMatcher( const std::string& eaname, long value )
        {
            return matcher_compare_numeric< long, greater_equal< long > >( eaname, value );
        }

        fh_matcher MakeGrEqDoubleMatcher( const std::string& eaname, double value )
        {
            return matcher_compare_numeric< double, greater_equal< double > >( eaname, value );
        }

        fh_matcher MakeGrEqBinaryMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_compare_case_sensitive_string< greater_equal< string > >( eaname, value );
        }

        fh_matcher MakeGrEqCISMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_compare_case_insensitive_string< greater_equal< string > >( eaname, value );
        }
        
        fh_matcher MakeTypeSafeGrEqMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_typesafe_greq( eaname, value );
        }
        
        /********************/
        /********************/
        /********************/

        fh_matcher MakeLtEqMatcher( const std::string& eaname, long value )
        {
            return matcher_compare_numeric< long, less_equal< long > >( eaname, value );
        }
        
        fh_matcher MakeLtEqDoubleMatcher( const std::string& eaname, double value )
        {
            return matcher_compare_numeric< double, less_equal< double > >( eaname, value );
        }

        fh_matcher MakeLtEqBinaryMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_compare_case_sensitive_string< less_equal< string > >( eaname, value );
        }

        fh_matcher MakeLtEqCISMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_compare_case_insensitive_string< less_equal< string > >( eaname, value );
        }
        
        fh_matcher MakeTypeSafeLtEqMatcher( const std::string& eaname, const std::string& value )
        {
            return matcher_typesafe_lteq( eaname, value );
        }

        /********************/
        /********************/
        /********************/
        
    };
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
/*
 * Create an object factory with id of token here.
 *
 * What if, binary function is a template that takes a pointer to a class
 * with
 * void setLeft( VirtaulMatcher* );
 * void setRight( VirtaulMatcher* );
 * VirtaulMatcher* operator();
 * fh_matcher style function 
 *
 */

    const string TOKENATTR = "token";
    fh_matcher ResolveFilter( fh_context filter );
    string getToken(  fh_context filter );


    namespace FilteredContextNameSpace
    {
        class BinaryOperationBase;
    };

    template < class Product >
    struct MakeObject
    {
        static FilteredContextNameSpace::BinaryOperationBase* Create()
            {
                return new Product();
            }
    };



//     typedef Loki::SingletonHolder<
//         Loki::Factory<
//         FilteredContextNameSpace::BinaryOperationBase,
//         string,
//         FilteredContextNameSpace::BinaryOperationBase* (*)()
//         >
//     >
//     OpFactory;
    typedef Loki::SingletonHolder<
        Loki::Factory< FilteredContextNameSpace::BinaryOperationBase, string >,
        Loki::CreateUsingNew, Loki::NoDestroy >
        OpFactory;


    namespace FilteredContextNameSpace
    {

    
        class FERRISEXP_DLLLOCAL UnEscapeLDAPFilterValueFailed : public FerrisVFSExceptionBase
        {
        public:
        
            inline UnEscapeLDAPFilterValueFailed(
                const FerrisException_CodeState& state,
                fh_ostream log,
                const string& e,
                Attribute* a=0)
                :
                FerrisVFSExceptionBase( state, log, e.c_str(), a )
                {
                    setExceptionName("UnEscapeLDAPFilterValueFailed");
                }
        };
#define Throw_UnEscapeLDAPFilterValueFailed(e,a) \
throw UnEscapeLDAPFilterValueFailed( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (e), (a))
    
    
        /*
         * Convert \2A in the input string into '*' in the output string.
         * Note that ctx is passed in for better debugging when exceptions
         * are thrown. It should be the context that is closest to where
         * the input was derived from.
         *
         * Char  Hex
         * '*'   0x2A 
         * '('   0x28
         * ')'   0x29
         * '\'   0x5C
         * '\0'  0x00
         *
         * Extensions to LDAP
         *
         * '~'   0x7E     (needed for =~ regex match)
         */
        FERRISEXP_DLLLOCAL string
        UnEscapeLDAPFilterValue( const string& input, const fh_context& ctx )
        {
            LG_FILTERPARSE_D << "UnEscapeLDAPFilterValue() input:" << input << endl;
        
            ostringstream oss;
            istringstream iss(input);
            int bytesRead = 0;
            char ch = 0;
            
            while( iss >> ch )
            {

                if( ch == '\\' )
                {
                    char ch1 = 0;
                    char ch2 = 0;

                    if( iss >> ch1 )
                    {
                        if( iss >> ch2 )
                        {
                            stringstream tss;
                            int v = 0;

                            if( ch1 >= '0' && ch1 <= '9' && ch2 >= '0' && ch2 <= '9' )
                            {
                                tss << ch1 << ch2;
                                tss >> v;
                                oss << (char)v;
                            }
                            else
                            {
                                oss << ch << ch1 << ch2;
                            }
                            bytesRead += 3;
                            continue;
                        }
                        else
                        {
                            ++bytesRead;
                            oss << ch1;
                            continue;
                        }
                    }
                    else
                    {
                        ++bytesRead;
                        oss << ch;
                        break;
                    }
                
                
//                 else
//                 {
//                     ostringstream ss;
//                     ss << "Error in UnEscapeLDAPFilterValue() at offset:" << bytesRead;
//                     Throw_UnEscapeLDAPFilterValueFailed(tostr(ss),GetImpl(ctx));
//                 }
                }
                ++bytesRead;
                oss << ch;
            }

            string output = tostr(oss);
            LG_FILTERPARSE_D << "UnEscapeLDAPFilterValue() output:" <<  output << endl;
            return output;
        }
    

    
        class FERRISEXP_DLLLOCAL BinaryOperationBase
        {
        private:

            fh_context L;
            fh_context R;
            fh_context filter;
            fh_istream orderedListStream;
        
            virtual fh_context getN()
                {
                    string name;
                    getline( orderedListStream, name );
    
                    LG_FILTERPARSE_D << " getN() :" << name << endl;
                    fh_context c = filter->getSubContext( name );
                    LG_FILTERPARSE_D << " c:" << c->getDirPath() << endl;

                    return c;
                }
        
        protected:
        
        public:
            BinaryOperationBase()
                :
                L(0), R(0), filter(0)
                {
                }
    
            void setFilter( const fh_context& v )
                {
                    filter = v;
                }
        

            void setLeft ( fh_context v )
                {
                    LG_FILTERPARSE_D << " setLeft() v->path:" << v->getDirPath() << endl;
                    L = v;
                }
    
            void setRight( fh_context v )
                {
                    R = v;
                }

            virtual void setIStream( const fh_istream& v )
                {
                    orderedListStream = v;
                }

        
        
            virtual fh_context getLeft()
                {
                    LG_FILTERPARSE_D << " getLeft(1) isBound: " << isBound(L) << endl;
                
                    if( isBound(L) )
                    {
                        return L;
                    }
                    setLeft(getN());
                
                    LG_FILTERPARSE_D << " getLeft(2) isBound: " << isBound(L) << endl;
//                L = getN();
                    LG_FILTERPARSE_D << " getLeft(3) isBound: " << isBound(L) << endl;
                    return L;
                }

            virtual fh_context getRight()
                {
                    if( !isBound(L) )
                    {
                        getLeft();
                    }

                    if( isBound(R) )
                    {
                        return R;
                    }
                    setRight(getN());
                    return R;
                }
        
            virtual fh_context getNextNode()
                {
                    if( !isBound(R) )
                    {
                        getRight();
                    }
                    return getN();
                }
        
    
            virtual fh_matcher operator()() = 0;
        };
    


        template < const string& TypeID, class TypeClass >
        class BinaryOperation : public BinaryOperationBase
        {
        protected:

            fh_context L;
            fh_context R;
    
        public:
            BinaryOperation()
                :
                L(0), R(0)
                {
                    OpFactory::Instance().Register( TypeID, &MakeObject<TypeClass>::Create );
                }
    

            virtual void setLeft ( const fh_context& v )
                {
                    L = v;
                }
    
            virtual void setRight( const fh_context& v )
                {
                    R = v;
                }
    
            virtual fh_matcher operator()() = 0;
        };

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string EqualsToken = "==";
        class FERRISEXP_DLLLOCAL EqualsOp : public BinaryOperation< EqualsToken, EqualsOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_context valc = getRight();
                    string key = getToken( getLeft()  );
                    string val = getToken( valc );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;

                    if( val == "*" )
                    {
                        return Factory::MakePresenceMatcher( key );
                    }

                    string comparisonOperator = guessComparisonOperatorFromData( val );

                    if( comparisonOperator == "string" )
                        return Factory::MakeEqualBinaryMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                    if( comparisonOperator == "cis" )
                        return Factory::MakeEqualCISMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                    if( comparisonOperator == "int" )
                        return Factory::MakeEqualIntegerMatcher(
                            key, toType<long>( UnEscapeLDAPFilterValue(val, valc)));
                    if( comparisonOperator == "double" || comparisonOperator == "float" )
                        return Factory::MakeEqualDoubleMatcher(
                            key, toType<double>( UnEscapeLDAPFilterValue(val, valc)));
                    return Factory::MakeEqualBinaryMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                    
                }
        };
        static EqualsOp EqualsOpObj;

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        
        const string TypeSafeEqualsToken = "=?=";
        class FERRISEXP_DLLLOCAL TypeSafeEqualsOp : public BinaryOperation< TypeSafeEqualsToken, TypeSafeEqualsOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_context valc = getRight();
                    string key = getToken( getLeft()  );
                    string val = getToken( valc );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;

                    if( val == "*" )
                    {
                        return Factory::MakePresenceMatcher( key );
                    }

                    return Factory::MakeTypeSafeEqualMatcher( key, UnEscapeLDAPFilterValue(val, valc) );
                }
        };
        static TypeSafeEqualsOp TypeSafeEqualsOpObj;
        
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string ReEqualsToken = "=~";
        class FERRISEXP_DLLLOCAL ReEqualsOp : public BinaryOperation< ReEqualsToken, ReEqualsOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    string val = "";
                    fh_context valc = 0;
                    
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;

                    try
                    {
                        LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                        valc = getRight();
                        val = getToken( valc );
                    }
                    catch( NoSuchSubContext& e )
                    {}

                    string key = getToken( getLeft()  );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;

//                 cerr << "Make a RE matcher for"
//                      << " key:" << key << " val:" << val << endl;

                    if( valc )
                        val = UnEscapeLDAPFilterValue(val, valc);
                    
                    return Factory::MakeRegexMatcher( key, val );
                }
        };
        static ReEqualsOp ReEqualsOpObj;

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string GrEqualsToken = ">=";
        class FERRISEXP_DLLLOCAL GrEqualsOp : public BinaryOperation< GrEqualsToken, GrEqualsOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_context valc = getRight();
                    string key = getToken( getLeft()  );
                    string val = getToken( valc );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;

                    string comparisonOperator = guessComparisonOperatorFromData( val );

                    if( comparisonOperator == "string" )
                        return Factory::MakeGrEqBinaryMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                    if( comparisonOperator == "cis" )
                        return Factory::MakeGrEqCISMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                    if( comparisonOperator == "int" )
                        return Factory::MakeGrEqMatcher(
                            key, toType<long>( UnEscapeLDAPFilterValue(val, valc)));
                    if( comparisonOperator == "double" || comparisonOperator == "float" )
                        return Factory::MakeGrEqDoubleMatcher(
                            key, toType<double>( UnEscapeLDAPFilterValue(val, valc)));
                    return Factory::MakeGrEqBinaryMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                }
        };
        static GrEqualsOp GrEqualsOpObj;

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string TypeSafeGrEqualsToken = ">?=";
        
        class FERRISEXP_DLLLOCAL TypeSafeGrEqualsOp : public BinaryOperation< TypeSafeGrEqualsToken, TypeSafeGrEqualsOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_context valc = getRight();
                    string key = getToken( getLeft()  );
                    string val = getToken( valc );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;

                    return Factory::MakeTypeSafeGrEqMatcher(
                        key, UnEscapeLDAPFilterValue(val, valc));
                }
        };
        static TypeSafeGrEqualsOp TypeSafeGrEqualsOpObj;


        
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string LtEqualsToken = "<=";
        class FERRISEXP_DLLLOCAL LtEqualsOp : public BinaryOperation< LtEqualsToken, LtEqualsOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_context valc = getRight();
                    string key = getToken( getLeft()  );
                    string val = getToken( valc );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;

                    string comparisonOperator = guessComparisonOperatorFromData( val );

//                     cerr << "LtEqualsOp compOp is:" << comparisonOperator
//                          << " value:" << val << endl;
                    
                    if( comparisonOperator == "string" )
                        return Factory::MakeLtEqBinaryMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                    if( comparisonOperator == "cis" )
                        return Factory::MakeLtEqCISMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                    if( comparisonOperator == "int" )
                        return Factory::MakeLtEqMatcher(
                            key, toType<long>( UnEscapeLDAPFilterValue(val, valc)));
                    if( comparisonOperator == "double" || comparisonOperator == "float" )
                        return Factory::MakeLtEqDoubleMatcher(
                            key, toType<double>( UnEscapeLDAPFilterValue(val, valc)));
                    return Factory::MakeLtEqBinaryMatcher( key, UnEscapeLDAPFilterValue(val, valc));
                }
        };
        static LtEqualsOp LtEqualsOpObj;

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string TypeSafeLtEqualsToken = "<?=";
        class FERRISEXP_DLLLOCAL TypeSafeLtEqualsOp : public BinaryOperation< TypeSafeLtEqualsToken, TypeSafeLtEqualsOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_context valc = getRight();
                    string key = getToken( getLeft()  );
                    string val = getToken( valc );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;

                    return Factory::MakeTypeSafeLtEqMatcher(
                        key, UnEscapeLDAPFilterValue(val, valc));
                }
        };
        static TypeSafeLtEqualsOp TypeSafeLtEqualsOpObj;

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string GrEqualsTokenFloatingPoint = ".>=";
        class FERRISEXP_DLLLOCAL GrEqualsOpFloatingPoint
            : public BinaryOperation< GrEqualsTokenFloatingPoint, GrEqualsOpFloatingPoint >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_context valc = getRight();
                    string key = getToken( getLeft()  );
                    string val = getToken( valc );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;
                    return Factory::MakeGrEqDoubleMatcher(
                        key, toType<double>(UnEscapeLDAPFilterValue(val, valc)));
                }
        };
        static GrEqualsOpFloatingPoint GrEqualsOpFloatingPointObj;

    
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string LtEqualsTokenFloatingPoint = ".<=";
        class FERRISEXP_DLLLOCAL LtEqualsOpFloatingPoint
            : public BinaryOperation< LtEqualsTokenFloatingPoint, LtEqualsOpFloatingPoint >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_context valc = getRight();
                    string key = getToken( getLeft()  );
                    string val = getToken( valc );
                    LG_FILTERPARSE_D << " key:" << key << " val:" << val << endl;

                    return Factory::MakeLtEqDoubleMatcher(
                        key, toType<double>( UnEscapeLDAPFilterValue(val, valc)));
                }
        };
        static LtEqualsOpFloatingPoint LtEqualsOpFloatingPointObj;
        
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string EndsWithToken = "~";
        class FERRISEXP_DLLLOCAL EndsWithOp : public BinaryOperation< EndsWithToken, EndsWithOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    string key = getToken( getLeft() );
                    string val = getToken( getRight() );
                    return Factory::MakeEndsWithMatcher( key, val );
                }
        };
        static EndsWithOp EndsWithOpObj;

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string NotToken = "!";
        class FERRISEXP_DLLLOCAL NotOp : public BinaryOperation< NotToken, NotOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    fh_matcher orig  = ResolveFilter( getLeft() );
                    return Factory::MakeNotMatcher( orig );
                }
        };
        static NotOp NotOpObj;

    
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
    
        const string AndToken = "&";
        class FERRISEXP_DLLLOCAL AndOp : public BinaryOperation< AndToken, AndOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_matcher leftm  = ResolveFilter( getLeft() );
                    fh_matcher rightm = ResolveFilter( getRight() );
                    fh_matcher ret = Factory::MakeAndMatcher( leftm, rightm );

                    try
                    {
                        while( true )
                        {
                            fh_matcher nextm = ResolveFilter( getNextNode() );
                            ret = Factory::MakeAndMatcher( ret, nextm );
                        }
                    }
                    catch( NoSuchSubContext& e )
                    {}
                
                    return ret;
                }
        };
        static AndOp AndOpObj;

        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

        const string OrToken = "|";
        class FERRISEXP_DLLLOCAL OrOp : public BinaryOperation< OrToken, OrOp >
        {
        public:
            virtual fh_matcher operator()()
                {
                    LG_FILTERPARSE_D << " OrOp" << endl;
                    LG_FILTERPARSE_D << " L:" << getLeft() ->getDirPath() << endl;
                    LG_FILTERPARSE_D << " R:" << getRight()->getDirPath() << endl;
                    fh_matcher leftm  = ResolveFilter( getLeft() );
                    fh_matcher rightm = ResolveFilter( getRight() );
                    fh_matcher ret = Factory::MakeOrMatcher( leftm, rightm );

                    try
                    {
                        while( true )
                        {
                            fh_matcher nextm = ResolveFilter( getNextNode() );
                            ret = Factory::MakeOrMatcher( ret, nextm );
                        }
                    }
                    catch( NoSuchSubContext& e )
                    {}
                
                    return ret;
                }
        };
        static OrOp OrOpObj;
    
    };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


    string
    getToken( fh_context filter )
    {
        LG_FILTERPARSE_D << " getToken() filter:" << filter->getDirPath() << endl;
//    LG_FILTERPARSE_D << " dirpath:" << filter->getDirPath() << endl;
//    LG_FILTERPARSE_D << " bound:" << isBound(filter) << endl;
    
        string token;
//    filter->dumpAttributeNames();
//     LG_FILTERPARSE_D << " TOKENATTR: " << TOKENATTR << endl;
//     LG_FILTERPARSE_D << " TOKENATTR: " << TOKENATTR << endl;

        try {
//        LG_FILTERPARSE_D << " going1" << endl;
            fh_attribute tattr = filter->getAttribute( TOKENATTR );
//        LG_FILTERPARSE_D << " going2" << endl;
        
            LG_FILTERPARSE_D << " getToken() tattr:" << tattr->getDirPath() << endl;
            fh_istream tss = tattr->getIStream();
            getline( tss, token );
            LG_FILTERPARSE_D << " getToken() token:" << token << endl;
        }
        catch( FerrisExceptionBase& e )
        {
            LG_FILTERPARSE_D << e.what() << endl;
        }
        catch( exception& e )
        {
            LG_FILTERPARSE_D << "gen:" << e.what() << endl;
        }
        catch(...)
        {
            LG_FILTERPARSE_D << "getToken() cought an unkown exception!" <<endl;
            throw;
        }
    
        return token;
    }

    
    fh_matcher
    ResolveFilter( fh_context filter )
    {
        CacheManager::fh_insideResolve resolveGuard = getCacheManager()->getInsideResolve();

        LG_FILTERPARSE_D << " resolving filter:" << filter->getDirPath() << endl;
    
        string token = getToken( filter );
        LG_FILTERPARSE_D << " token:" << token << endl;
    
        FilteredContextNameSpace::BinaryOperationBase* bo =
            OpFactory::Instance().CreateObject( token );

        LG_FILTERPARSE_D << " filter:" << filter->getDirPath() << endl;
//         filter->dumpAttributeNames();
//         LG_FILTERPARSE_D << " filter:" << filter->getDirPath() << endl;

    
        fh_attribute inorderlist = filter->getAttribute( "in-order-insert-list" );

//     string leftname;
//     string rightname;
        fh_istream inorderlistss = inorderlist->getIStream();
        bo->setFilter( filter );
        bo->setIStream( inorderlistss );
    
    
//     getline( inorderlistss, leftname );
//     getline( inorderlistss, rightname );
    
//     LG_FILTERPARSE_D << " leftname :" << leftname << endl;
//     LG_FILTERPARSE_D << " rightname:" << rightname << endl;
//     fh_context leftc = filter->getSubContext( leftname );
//     LG_FILTERPARSE_D << " leftc:" << leftc->getDirPath() << endl;
//     bo->setLeft ( leftc );
//     LG_FILTERPARSE_D << " filter:" << filter->getDirPath() << endl;
//     bo->setRight( filter->getSubContext( rightname ));
//     LG_FILTERPARSE_D << " filter:" << filter->getDirPath() << endl;

        fh_matcher ret = bo->operator()();
        delete bo;
        return ret;
    }



    namespace Factory
    {
        
        fh_filtContext FilteredView( const fh_context& ctx, const fh_matcher& matcher )
        {
            return new FilteredContext( ctx, matcher );
        }

        fh_matcher MakeMatcherFromContext( fh_context f )
        {
            CacheManager::fh_insideResolve resolveGuard = getCacheManager()->getInsideResolve();

            f->read();
            ContextCollection::SubContextNames_t names = f->getSubContextNames();
            fh_context root_of_filter = f->getSubContext( *(names.begin()) );
            fh_matcher filt = ResolveFilter( root_of_filter );
            return filt;
        }

        fh_context MakeFilteredContext( fh_context& ctx, const std::string& filterString )
        {
            fh_context filter = Factory::MakeFilter( filterString );
            return Factory::MakeFilteredContext( ctx, filter );
        }
        
        fh_context MakeFilteredContext( fh_context& ctx, fh_context& f )
        {
            CacheManager::fh_insideResolve resolveGuard = getCacheManager()->getInsideResolve();

            LG_FILTERPARSE_D << " f:" << f->getDirPath() << endl;

            f->read();
        
            LG_FILTERPARSE_D << " SubContextCount: " << f->SubContextCount() << endl;

            ContextCollection::SubContextNames_t names = f->getSubContextNames();

//         cerr << "--- MakeFilteredContext() ---" << endl;
//         for( ContextCollection::SubContextNames_t::iterator iter = names.begin();
//              iter != names.end();
//              ++iter )
//         {
//             cerr << "--- iter:" << *iter << endl;
//         }
//         cerr << "--- MakeFilteredContext() ---" << endl;
//         cerr << " root_of_filter name:" << f->getDirName() << endl;
//         cerr << " root_of_filter path:" << f->getDirPath() << endl;
//         cerr << " root_of_filter rdn:" << *(names.begin()) << endl;
            LG_FILTERPARSE_D << " root_of_filter rdn:" << *(names.begin()) << endl;

        
            fh_context root_of_filter = f->getSubContext( *(names.begin()) );
            LG_FILTERPARSE_D << " root_of_filter:" << root_of_filter->getDirPath() << endl;

//             cerr << "MakeFilteredContext() f:" << f->getURL()
//                  << " f->staticstr:" << getStrAttr( f, "static-string", "" ) << endl;
            
            fh_matcher filt = ResolveFilter( root_of_filter );

            LG_FILTERPARSE_D << " got matcher" << endl;


            FilteredContext* retfc = new FilteredContext( ctx, filt );
            
            fh_context ret;
            Upcast( ret, retfc );
            retfc->setup();

            retfc->setIsChainedViewContextRoot();

//            cerr << "Setting filter EA to:" << getStrAttr( f, "static-string", "" ) << endl;
            bool rc = retfc->addAttribute( "filter",
                                           getStrAttr( f, "static-string", "" ),
                                           FXD_FFILTER );

            retfc->addAttribute( "in-order-insert-list",
                                 getStrAttr( f, "in-order-insert-list", "" ),
                                 FXD_STRINGLIST );
    
//             cerr << "AddAttribute rc:" << rc << endl;
//             cerr << "Getting filter EA:" << getStrAttr( retfc, "filter", "no" ) << endl;
            
//             {
//                 string v =  "x value";
//                 string k = "keyXXXXXXXX";
                
//                 retfc->addAttribute( k, v );
//                 cerr << "Getting test EA:" << getStrAttr( retfc, k, "no2" ) << endl;
                
//             }
            
            return ret;
        }

        fh_context MakeFilter( const string& v_const )
        {
            string v = v_const;

            if( v.empty() )
            {
                v = "(name=~.*)";
            }
            
            try
            {
                CacheManager::fh_insideResolve resolveGuard = getCacheManager()->getInsideResolve();
            
                RootContextFactory fac;

                LG_FILTERPARSE_D << " filter:" << v << endl;

                fac.setContextClass( "ffilter" );
                fac.AddInfo( RootContextFactory::ROOT, "/" );
                fac.AddInfo( "StaticString", v );

                LG_FILTERPARSE_D << " resolving(1) filter:" << v << endl;
                fh_context c = fac.resolveContext( RESOLVE_EXACT );
                LG_FILTERPARSE_D << " resolving(2) filter:" << v << endl;
                LG_FILTERPARSE_D << " resolving(2) c:" << c->getDirPath() << endl;
                LG_FILTERPARSE_D << " resolving(3)" << endl;

                c->read();
                return c;
            }
            catch( CanNotReadContextPcctsParseFailed& e )
            {
                stringstream ss;
                const stringlist_t& sl = e.getSyntaxErrorList();
                ss << "Syntax error for ffilter:" << endl
                   << v << endl;
                for( stringlist_t::const_iterator si = sl.begin(); si!=sl.end(); ++si )
                {
                    ss << *si << endl;
                }
                Throw_FFilterSyntaxError( tostr(ss), 0 );
            }
            catch( ... )
            {
                throw;
            }
        }


        fh_context MakeFilterFromFile( const string& v )
        {
            CacheManager::fh_insideResolve resolveGuard = getCacheManager()->getInsideResolve();

            RootContextFactory fac;

//         cerr << " filter:" << v << endl;

            fac.setContextClass( "ffilter" );
            fac.AddInfo( RootContextFactory::ROOT, v );
            fac.AddInfo( RootContextFactory::PATH, ""   );
            fh_context c = fac.resolveContext( RESOLVE_EXACT );
            c->read();
            return c;
        }
    };

    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*** Filtered Context **********************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    /*******************************************************************************/
    
    void
    FilteredContext::setupState( const fh_context& ctx, fh_matcher m )
    {
        Delegate = ctx;
        Matcher = m;
    }
    
/**
 * Create a filtered context that uses the base context ctx and only exposes
 * the contexts from the base context that return true under the predicate
 * (matcher). Note that one *must* call setup() after this method for correct
 * operation.
 *
 * @param ctx Base context
 * @param matcher Predicate that must == true for each context from base to be shown
 */
    FilteredContext::FilteredContext(
        const fh_context& ctx,
        fh_matcher matcher )
        :
        ChainedViewContext(ctx, false),
        Matcher( matcher )
    {
        LG_FILTERPARSE_D << "======= FilteredContext::FilteredContext() ==========" << endl;
    }

    FilteredContext::FilteredContext( Context* theParent,
                                      const fh_context& ctx,
                                      fh_matcher matcher )
        :
        ChainedViewContext(ctx, false),
        Matcher( matcher )
    {
        setContext( theParent, ctx->getDirName() );
    }
    
    
    FilteredContext::~FilteredContext()
    {
        LG_VM_D << "~FilteredContext() this:" << toVoid(this) << endl;
    }
    

    void
    FilteredContext::populateFromDelegate()
    {
        LG_CTX_D << "FilteredContext::populateFromDelegate() url:" << getURL() << endl;
        
        Context::iterator ci = Delegate->begin();
        Context::iterator e  = Delegate->end();

        for( ; ci != e ; ++ci )
        {
            try
            {
                filteringInsertContext( *ci );
            }
            catch(NoSuchSubContext& e)
            {
                LG_CTX_ER << "FilteredContext::FilteredContext() "
                          << "Context:" << ci->getURL()
                          << " advertised but not presentable!"
                          << " e:" << e.what()
                          << endl;
            }
        }
        
        
//         SubContextNames_t ls = Delegate->getSubContextNames();

//         for( SubContextNames_t::iterator iter = ls.begin();
//              iter != ls.end(); iter++ )
//         {
//             try {
//                 LG_FILTERPARSE_D << "Testing name:" << *iter << endl;
//                 fh_context c = Delegate->getSubContext( *iter );
//                 filteringInsertContext( c );
//             }
//             catch(NoSuchSubContext& e)
//             {
//                 LG_CTX_ER << "FilteredContext::FilteredContext() "
//                           << "Context:" << *iter
//                           << " advertised but not presentable!"
//                           << " e:" << e.what()
//                           << endl;
//             }
//         }
    }
    
    
/** 
 * Perform initial setup of items based on the matching predicate. Note that the
 * caller *MUST* hold a reference to the object for this call to work.
 */
    void
    FilteredContext::setup()
    {
        LG_FILTERPARSE_D << "======= FilteredContext::setup() ==========" << endl;

//         cerr << "======= FilteredContext::setup() ==========" << endl;
//         BackTrace();
        
        populateFromDelegate();
        SetupEventConnections();
    }



    void
    FilteredContext::read( bool force )
    {
        if( ReadingDir )
            return;

        ReadingDirRAII __raii1( this, true );
        LG_CTX_D << "FilteredContext::read(starting) c:" << getURL() << endl;
        
//         if( !HaveReadDir )
//         {
//             LG_CTX_D << "FilteredContext::read(1) c:" << getURL() << " sz:" << getItems().size() << endl;
//             setup();
//             HaveReadDir = true;
//             LG_CTX_D << "FilteredContext::read(1.b) c:" << getURL() << " sz:" << getItems().size() << endl;
//         }
//         else
        {
            ensureEventConnections();
            
            if( isActiveView() && !getItems().empty() )
            {
                LG_CTX_D << "FilteredContext::read(2) c:" << getURL() << " sz:" << getItems().size() << endl;
                emitExistsEventForEachItem();
            }
            else
            {
                LG_CTX_D << "FilteredContext::read(3) c:" << getURL()
                         << " sz:" << getItems().size()
                         << " Delegate.sz:" << Delegate->getItems().size()
                         << " local-version:" << getDirOpVersion()
                         << " delegate-version:" << Delegate->getDirOpVersion()
                         << endl;
                if( getDirOpVersion() < Delegate->getDirOpVersion() )
                {
                    clearContext();
                    populateFromDelegate();
                }
                else
                {
                    emitExistsEventForEachItem();
                }
            }
        }

        LG_CTX_D << "FilteredContext::read(complete) c:" << getURL() << endl;
    }
    


/**
 * When a context is deleted from the base context it also gets removed from the filtering
 * context.
 */
    void
    FilteredContext::OnDeleted( NamingEvent_Deleted* ev, string olddn, string newdn )
    {
        Emit_Deleted( ev, newdn, olddn, 0 );
        Remove( ev->getSource()->getSubContext( olddn ) );
    }


        
    FilteredContext::filteringInsertContextCreator::filteringInsertContextCreator( const fh_context& ctx,
                                                                                   fh_matcher matcher )
        :
        m_ctx( ctx ),
        m_matcher( matcher )
    {
    }
    FilteredContext*
    FilteredContext::filteringInsertContextCreator::create( Context* parent, const std::string& rdn ) const
    {
        return new FilteredContext( parent, m_ctx, m_matcher );
    }
    void
    FilteredContext::filteringInsertContextCreator::setupExisting( FilteredContext* fc ) const
    {
        fc->setupState( m_ctx, m_matcher );
    }
    void
    FilteredContext::filteringInsertContextCreator::setupNew( FilteredContext* fc ) const
    {
//        fc->SetupEventConnections();
    }
    


/**
 * If the matcher given at FilteredContext creation time allows the given context
 * to be inserted then the context is inserted into this view, otherwise there
 * is no action taken.
 *
 * @param c Context that may be inserted if it passes the filtering matcher.
 */
    void
    FilteredContext::filteringInsertContext( const fh_context& c, bool created )
    {
        
        LG_FILTERPARSE_D << "Filtering context:" << c->getDirPath() << endl;
//        cerr << "Filtering context:" << c->getDirPath() << endl;
//        BackTrace();
        
        if( Matcher(c) )
        {
            LG_FILTERPARSE_D << "PASS FOR Filtering context:" << c->getDirPath() << endl;

            FilteredContext* fc = 0;
            fc = priv_ensureSubContext( c->getDirName(), fc,
                                        filteringInsertContextCreator( c, Matcher ) );
        }
        else
        {
            LG_FILTERPARSE_D << "FAIL FOR Filtering context:" << c->getDirPath() << endl;
        }
    }

    void
    FilteredContext::UnPageSubContextsIfNeeded()
    {
//        cerr << "FilteredContext::UnPageSubContextsIfNeeded()" << endl;
        if( isBound( Delegate ) )
        {
            Delegate->UnPageSubContextsIfNeeded();
        }
        
        return;
    }
    
    void
    FilteredContext::OnCreated( NamingEvent_Created* ev,
                                const fh_context& subc,
                                std::string olddn, std::string newdn )
    {
        LG_CTX_D << "FilteredContext::OnCreated() " << endl;
        filteringInsertContext( subc, true );
    }


/**
 * Add the newly discovered context to the view if it passes the matcher.
 */
    void
    FilteredContext::OnExists ( NamingEvent_Exists* ev,
                                const fh_context& subc,
                                string olddn, string newdn )
    {
        LG_CTX_I << "FilteredContext::OnExists() " << endl;
        filteringInsertContext( subc );
    }


/**
 * Disallow and log any attempt to directly create a new context.
 * All other methods should delegate the creation of new subcontexts to
 * the underlying base context and from there the events will inform
 * this context of the creation and we will in turn filter that new
 * context.
 */
    Context*
    FilteredContext::priv_CreateContext( Context* parent, string rdn )
    {
        LG_CTX_ER << "priv_CreateContext() should never happen" << endl;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    int
    FilteredContext::getNumberOfLocalAttributes()
    {
        return getLocalAttributeNames().size();
    }
    
    list< string >&
    FilteredContext::getLocalAttributeNames()
    {
        static list< string > ret;
        if( ret.empty() )
        {
            ret.push_back( "filter" );
            ret.push_back( "in-order-insert-list" );
        }
        return ret;
    }
    

    std::string
    FilteredContext::private_getStrAttr( const std::string& rdn,
                                         const std::string& def,
                                         bool getAllLines,
                                         bool throwEx )
    {
//         cerr << "FilteredContext::private_getStrAttr() rdn:" << rdn
//              << " local:" << ( find( getLocalAttributeNames().begin(), getLocalAttributeNames().end(), rdn )
//             != getLocalAttributeNames().end() )
//              << " OMCD:" << isBound(OverMountContext_Delegate)
//              << " base::isBound:" << _Base::isAttributeBound( rdn )
//              << " ctx::isBound:" << Context::isAttributeBound( rdn )
//              << " del::isBound:" << Delegate->isAttributeBound( rdn )
//              << endl;
        
        if( find( getLocalAttributeNames().begin(), getLocalAttributeNames().end(), rdn )
            != getLocalAttributeNames().end() )
            return Context::private_getStrAttr( rdn, def, getAllLines, throwEx );
        return Delegate->private_getStrAttr( rdn, def, getAllLines, throwEx );
    }
    
    fh_attribute
    FilteredContext::getAttribute( const string& rdn ) throw( NoSuchAttribute )
    {
        if( find( getLocalAttributeNames().begin(), getLocalAttributeNames().end(), rdn )
            != getLocalAttributeNames().end() )
        {
            return Context::getAttribute(rdn);
        }
        return Delegate->getAttribute(rdn);
    }
    
    AttributeCollection::AttributeNames_t&
    FilteredContext::getAttributeNames( AttributeNames_t& ret )
    {
        AttributeCollection::AttributeNames_t t1;
        AttributeCollection::AttributeNames_t t2;
        Delegate->getAttributeNames( t1 );
        copy( getLocalAttributeNames().begin(),
              getLocalAttributeNames().end(),
              back_inserter( t2 ) );
        return mergeAttributeNames( ret, t1, t2 );
    }
    
    int
    FilteredContext::getAttributeCount()
    {
        return getNumberOfLocalAttributes() + Delegate->getAttributeCount();
    }

    bool
    FilteredContext::isAttributeBound( const std::string& rdn,
                                       bool createIfNotThere
        ) throw( NoSuchAttribute )
    {
        if( find( getLocalAttributeNames().begin(), getLocalAttributeNames().end(), rdn )
            != getLocalAttributeNames().end() )
            return Context::isAttributeBound( rdn, createIfNotThere );
        return Delegate->isAttributeBound( rdn, createIfNotThere );
    }    
    
};

