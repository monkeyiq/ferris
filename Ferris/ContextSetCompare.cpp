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

    $Id: ContextSetCompare.cpp,v 1.9 2010/11/19 21:30:13 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris.hh>
#include <ContextSetCompare_private.hh>
#include <Context_private.hh>
#include <Trimming.hh>

#include "FSParser_private.hh"

using namespace std;

namespace Ferris
{

    static std::string
    adjustEANameForSorting_adjustTime( const std::string& cn,
                                       const std::string& prefix,
                                       const std::string& replacement )
    {
        if( starts_with( cn, prefix ))
        {
            return replacement;
        }
        return cn;
    }
    
    

    
    std::string
    adjustEANameForSorting( const std::string& s )
    {
        std::string cn = s;

        if( starts_with( cn, "size" ))
        {
            cn = "size";
        }
        cn = adjustEANameForSorting_adjustTime( cn, "mtime-", "mtime" );
        cn = adjustEANameForSorting_adjustTime( cn, "atime-", "atime" );
        cn = adjustEANameForSorting_adjustTime( cn, "ctime-", "ctime" );
        if( ends_with( cn, "-pixbuf" ))
        {
            cn = cn.substr( 0, cn.length() - strlen("-pixbuf"));
        }
        
        return cn;
    }

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    PreprocessedSortString::PreprocessedSortString( const std::string& s )
        :
        SortString( s )
    {
    }
    
    std::string PreprocessedSortString::getString()
    {
        return SortString;
    }

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    /* Compare S1 and S2 as strings holding name & indices/version numbers.  */
    extern int strverscmp (__const char *__s1, __const char *__s2)
        __THROW __attribute_pure__;
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    struct FERRISEXP_DLLLOCAL ContextSetCompare_FunctorData
    {
        string eaname;
        ContextSetCompare_FunctorData( string eaname )
            :
            eaname( eaname )
            {
            }
    };
    
    struct FERRISEXP_DLLLOCAL ContextSetComparePData
    {
        typedef Loki::Functor< bool, LOKI_TYPELIST_2( const fh_context&,
                                                      const fh_context& ) > Fun_t;
        Fun_t fun;
        /**
         * 0 is sort by name, ie. default. otherwise it points to a string
         * that shouldn't be deallocated ever
         */
        const char* m_sortString;
        
        ContextSetComparePData()
            :
            m_sortString( 0 )
            {}
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /*
     * Note that these functions being used in the set<> for Context::Items_t should
     * not throw exceptions but quietly return default data for streams that fail to
     * provide information
     */
    
    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_reverse
    {
        ContextSetCompare_FunctorData d;
        ContextSetComparePData::Fun_t innerFunc;
        
        inline contextset_reverse( const ContextSetCompare_FunctorData& d,
                                   ContextSetComparePData::Fun_t innerFunc )
            :
            d( d ),
            innerFunc( innerFunc )
            {
            }
        
        bool operator()( const _Tp& xc, const _Tp& yc)
            {
                bool res = innerFunc( yc, xc );
//                cerr << "reverseop() ea:" << d.eaname << " res:" << res << endl;
                return res;
            }
    };

    /** 
     * Chain together sorting operations. If the 'current' predicate tests
     * that xc and yc are Equivalent (in the tech sense) then we chain to
     * the next sorter to make sure that we get an total order.
     *
     * We can be sure of a total order because the last item in the chain
     * should be a 'name' based sort like :CIS:name or name and because
     * there can be no two subcontexts with the same name ordering is
     * ensured.
     */
    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_cascade
    {
        ContextSetCompare_FunctorData d;
        ContextSetComparePData::Fun_t current;
        ContextSetComparePData::Fun_t next;
        
        contextset_cascade(
            const ContextSetCompare_FunctorData& d,
            ContextSetComparePData::Fun_t current,
            ContextSetComparePData::Fun_t next )
            :
            d( d ),
            current( current ),
            next( next )
            {
            }
        
        bool operator()( const _Tp& xc, const _Tp& yc)
            {
                bool res = current( xc, yc );
//                cerr << "cascade() ea:" << d.eaname << " res:" << res << endl;
                if( res )
                {
                    /* xc is definately smaller than yc */
                    return res;
                }
                bool revres = current( yc, xc );

                /* Test for Equivalence */
                if( !res && !revres )
                {
                    /* They are the same in the eyes of this sorter, chain to
                     * the next link */
                    return next( xc, yc );
                }

                /* They are not Equivalent, yc > xc by this sorter */
                return res;
            }
    };
    
    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_less_strverscmp
    {
        ContextSetCompare_FunctorData d;
        contextset_less_strverscmp( const ContextSetCompare_FunctorData& d )
            :
            d( d )
            {}
        
        bool operator()( const _Tp& xc, const _Tp& yc) const
            {
                string x = getStrAttr( xc, d.eaname, "" );
                string y = getStrAttr( yc, d.eaname, "" );

                int rc = ::strverscmp( x.c_str(), y.c_str() );
//                 cerr << "strverscmp eaname:" << d.eaname
//                      << " x:" << x
//                      << " y:" << y
//                      << " res:" << ((rc < 0) ? true : false)
//                      << endl;
                return (rc < 0) ? true : false;
            }
    };

    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_less_fscmp_base
    {
        ContextSetCompare_FunctorData d;
        contextset_less_fscmp_base( const ContextSetCompare_FunctorData& d )
            :
            d( d )
            {}

        typedef set< char > charset_t;
        charset_t& getDropSet() const
            {
                static charset_t ret;
                if( ret.empty() )
                {
                    ret.insert('-');
                    ret.insert(' ');
                    ret.insert('_');
                }
                return ret;
            }
        
        string clean( const std::string& s ) const
            {
                stringstream iss;
                stringstream oss;
                iss << s;
                char ch;
                const charset_t& ds = getDropSet();
                charset_t::const_iterator ds_end = ds.end();

                bool ignore_chars = false;
                while( iss >> ch )
                {
                    if( ds_end != ds.find( ch ) )
                    {
                    }
                    else if( ch == '[' )
                    {
                        ignore_chars = true;
                    }
                    else if( ch == ']' )
                    {
                        ignore_chars = false;
                    }
                    else
                    {
                        if( ignore_chars )
                        {
                        }
                        else
                        {
                            oss << ch;
                        }
                    }
                }
                return oss.str();
            }

        bool operator()( const _Tp& xc, const _Tp& yc, bool check ) const
            {
                string x = getStrAttr( xc, d.eaname, "" );
                string y = getStrAttr( yc, d.eaname, "" );

                try
                {
                    FSParser px;
                    FSParser py;
                    bool px_ok = px.parse( x );
                    bool py_ok = py.parse( y );
                    
                    if( px_ok && py_ok )
                    {
                        if( check )
                        {
                            string xn = tolowerstr()( px.getName() );
                            string yn = tolowerstr()( py.getName() );
                            int n_min = min( xn.length(), yn.length() );
                        
                            if( xn.substr( 0, n_min ) == yn.substr( 0, n_min ) )
                            {
                                if( px.getE() == py.getE() )
                                    return 0;
                                return px.getE() < py.getE();
                            }
                        }
                        else
                        {
                            string xn = tolowerstr()( px.getName() );
                            string yn = tolowerstr()( py.getName() );
                            int n_min = min( xn.length(), yn.length() );

//                             cerr << "xn:" << xn << endl
//                                  << "yn:" << yn << endl
//                                  << "   x:" << px.getE() << " y:" << py.getE() << endl << endl;
                            
                            if( px.getE() == py.getE() )
                                return 0;
                            return px.getE() < py.getE();
                        }
                    }
                    if( px_ok || py_ok )
                    {
                        if( px_ok )
                            return -1;
                        else
                            return 1;
                    }
                }
                catch( exception& e )
                {
                }
                    
                x = clean( x );
                y = clean( y );
                    
                int rc = ::strverscmp( x.c_str(), y.c_str() );
                return (rc < 0) ? true : false;
            }
    };


    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_less_fscmp
        :
        public contextset_less_fscmp_base< _Tp >
    {
        typedef contextset_less_fscmp_base< _Tp > _Base;
        using _Base::d;
        using _Base::clean;
        
        contextset_less_fscmp( const ContextSetCompare_FunctorData& d )
            :
            _Base( d )
            {}

        bool operator()( const _Tp& xc, const _Tp& yc ) const
            {
                return _Base::operator()( xc, yc, true );
            }
    };

    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_less_fscmp_nocheck
        :
        public contextset_less_fscmp_base< _Tp >
    {
        typedef contextset_less_fscmp_base< _Tp > _Base;
        using _Base::d;
        using _Base::clean;
        
        contextset_less_fscmp_nocheck( const ContextSetCompare_FunctorData& d )
            :
            _Base( d )
            {}

        bool operator()( const _Tp& xc, const _Tp& yc ) const
            {
                return _Base::operator()( xc, yc, false );
            }
        
    };
    

    
    
    /**
     * Compares using long ints and numeric <
     */
    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_less_numeric
    {
        ContextSetCompare_FunctorData d;
        contextset_less_numeric( const ContextSetCompare_FunctorData& d )
            :
            d( d )
            {}

        bool operator()( const _Tp& xc, const _Tp& yc) const
            {
//                 long x = 0;
//                 long y = 0;
//                 fh_attribute xa = xc->getAttribute( d.eaname );
//                 fh_attribute ya = yc->getAttribute( d.eaname );
//                 fh_istream   xi = xa->getIStream();
//                 fh_istream   yi = ya->getIStream();
//                 xi >> x;
//                 yi >> y;

                long x = toType<long>( getStrAttr( xc, d.eaname, "0" ));
                long y = toType<long>( getStrAttr( yc, d.eaname, "0" ));
                
//                 cerr << "numeric_compare()"
//                      << " xi >> x:" << (xi >> x).good()
//                      << " yi >> y:" << (yi >> y).good()
//                      << endl;

//                 cerr << "numeric_compare ea:" << d.eaname
//                      << " xname:" << xc->getDirName()
//                      << " yname:" << yc->getDirName()
//                      << " x:" << x
//                      << " y:" << y
//                      << " res:" << (x<y)
//                      << endl;

                return y < x;
            }
    };


    /**
     * Compares using long doubles and numeric <
     */
    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_less_floating_numeric
    {
        ContextSetCompare_FunctorData d;
        contextset_less_floating_numeric( const ContextSetCompare_FunctorData& d )
            :
            d( d )
            {}

        bool operator()( const _Tp& xc, const _Tp& yc) const
            {
                typedef long double numT;
                numT x = toType<numT>( getStrAttr( xc, d.eaname, "0" ));
                numT y = toType<numT>( getStrAttr( yc, d.eaname, "0" ));
                return y < x;
            }
    };
    
    /**
     * Sorts the existed items before the created items.
     * Note that this is the sorter that is used to create 'lazy' sorting
     * even though there is actually more work being done to sort new items
     * at the bottom of the order.
     */
    struct FERRISEXP_DLLLOCAL contextset_less_created_or_existed
    {
        ContextSetCompare_FunctorData d;
        contextset_less_created_or_existed( const ContextSetCompare_FunctorData& d )
            :
            d( d )
            {}

        bool operator()( const fh_context& xc, const fh_context& yc) const
            {
//                 cerr << "lazy_compare"
//                      << " xname:" << xc->getDirName()
//                      << " yname:" << yc->getDirName()
//                      << " x:" << xc->ContextWasCreatedNotDiscovered
//                      << " y:" << yc->ContextWasCreatedNotDiscovered
//                      << " res:" << ( xc->ContextWasCreatedNotDiscovered
//                                     < yc->ContextWasCreatedNotDiscovered )
//                      << endl;
                return xc->ContextWasCreatedNotDiscovered
                    <  yc->ContextWasCreatedNotDiscovered;
            }
    };

    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_less_string
    {
        ContextSetCompare_FunctorData d;
        contextset_less_string( const ContextSetCompare_FunctorData& d )
            :
            d( d )
            {}

        bool operator()( const _Tp& xc, const _Tp& yc) const
            {
//                 cerr << "normal compare() ea:" << d.eaname
//                      << " x:" << getStrAttr( xc, d.eaname, "" )
//                      << " y:" << getStrAttr( yc, d.eaname, "" )
//                      << " res:" << (getStrAttr( xc, d.eaname, "" )<getStrAttr( yc, d.eaname, "" ))
//                      << endl;
                return getStrAttr( xc, d.eaname, "" )
                    <  getStrAttr( yc, d.eaname, "" );
            }
    };

    template <class _Tp>
    struct FERRISEXP_DLLLOCAL contextset_less_cis_string
    {
        ContextSetCompare_FunctorData d;
        contextset_less_cis_string( const ContextSetCompare_FunctorData& d )
            :
            d( d )
            {}

        inline Nocase& getCmp() const
            {
                static Nocase nocasefun;
                return nocasefun;
            }
        
        bool operator()( const _Tp& xc, const _Tp& yc) const
            {
                return getCmp()( getStrAttr( xc, d.eaname, "" ),
                                 getStrAttr( yc, d.eaname, "" ) );
            }
    };

    struct FERRISEXP_DLLLOCAL contextset_rdn
    {
        contextset_rdn() 
            {}

        inline bool operator()( const fh_context& xc, const fh_context& yc) const
            {
                return xc->getDirName() < yc->getDirName();
            }
    };
    
    
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    ContextSetCompare::ContextSetCompare( const std::string& s )
        :
        d( new ContextSetComparePData() )
    {
        setup( s );
    }

    ContextSetCompare::ContextSetCompare( fh_sorter s )
        :
        d( new ContextSetComparePData() )
    {
        setup( s->getString() );
    }
    
    ContextSetCompare::ContextSetCompare( const ContextSetCompare& ci )
        :
        d( new ContextSetComparePData() )
    {
        d->fun     = ci.d->fun;
    }
    
    ContextSetCompare&
    ContextSetCompare::operator=( const ContextSetCompare& ci )
    {
        d->fun     = ci.d->fun;
        return *this;
    }

    ContextSetCompare::~ContextSetCompare()
    {
        if( d ) delete d;
    }


    static
    ContextSetComparePData::Fun_t
    createSortFunction( const std::string& s )
    {
        typedef ContextSetComparePData::Fun_t Fun_t;
        Fun_t ret;
        
        bool numeric = false;
        bool floatingpoint = false;
        bool caseSensitive = true;
        bool versionCompare = false;
        bool fsCompare = false;
        bool fsCompareNoCheck = false;
        bool reverse = false;
        bool lazy = false;
        string AttrName = s;

        /*
         * Break out options
         */
        if( s.length() > 2 && s[0] == ':' )
        {
            int optionsStartPos = 1;
                
            if( s[1] == '!' )
            {
                ++optionsStartPos;
                reverse = true;
            }
            if( s[1] == 'L' || (s.length() > 2 && s[2] == 'L') )
            {
                ++optionsStartPos;
                lazy = true;
            }
                
            string::size_type en = s.find( ":", optionsStartPos );
            if( en == string::npos )
            {
                stringstream ss;
                ss << "Unclosed meta info area (no second ':' found)"
                   << " string:" << s;
                Throw_InvalidSortSpecification( tostr(ss), 0 );
            }
                
            string type = s.substr( optionsStartPos, en-optionsStartPos );
            AttrName    = s.substr( en+1 );

            LG_SORT_D
                << " s:" << s
                << " optionsStartPos:" << optionsStartPos
                << " en:" << en 
                << " type:" << type 
                << " AttrName:" << AttrName
                << endl;
                
            if( type == "#" )
            {
                numeric = true;
            }
            else if( type == "FLOAT" )
            {
                floatingpoint = true;
            }
            else if( type == "CIS" )
            {
                caseSensitive = false;
            }
            else if( type == "VER" || type == "V" )
            {
                versionCompare = true;
            }
            else if( type == "f" || type == "funsub" )
            {
                fsCompare = true;
            }
            else if( type == "F" || type == "FUNSUB" )
            {
                fsCompareNoCheck = true;
            }
            else
            {
                LG_SORT_D << "Ignoring some meta tags:" << type << endl;
            }
        }

        LG_SORT_D << "MakeSorter() "
                  << " s:" << s
                  << " numeric:" << numeric
                  << " caseSensitive:" << caseSensitive
                  << " reverse:" << reverse
                  << " vc:" << versionCompare
                  << " fc:" << fsCompare
                  << " FC:" << fsCompareNoCheck
                  << " lazy:" << lazy
                  << endl;
//         cerr << "MakeSorter() "
//              << " s:" << s
//              << " numeric:" << numeric
//              << " caseSensitive:" << caseSensitive
//              << " reverse:" << reverse
//              << " vc:" << versionCompare
//              << " lazy:" << lazy
//              << endl;

        /*
         * Make sorter based on options given
         */
        if( numeric )
        {
            LG_SORT_D << "Making a numeric sorter for s:" << s
                      << " AttrName:" << AttrName
                      << endl;
//             cerr << "Making a numeric sorter for s:" << s
//                  << " AttrName:" << AttrName
//                  << endl;
            if( floatingpoint )
                ret = contextset_less_floating_numeric<fh_context>( AttrName );
            else
                ret = contextset_less_numeric<fh_context>( AttrName );
        }
        else if( versionCompare )
        {
            LG_SORT_D << "Making a version sorter for s:" << s
                      << " AttrName:" << AttrName
                      << endl;
            ret = contextset_less_strverscmp<fh_context>( AttrName );
        }
        else if( fsCompare )
        {
            ret = contextset_less_fscmp<fh_context>( AttrName );
        }
        else if( fsCompareNoCheck )
        {
            ret = contextset_less_fscmp_nocheck<fh_context>( AttrName );
        }
        else if( caseSensitive )
        {
            LG_SORT_D << "Making a case sensitive sorter for s:" << s
                      << " AttrName:" << AttrName
                      << endl;
            ret = contextset_less_string<fh_context>( AttrName );
        }
        else
        {
            LG_SORT_D << "Making a case NOT sensitive sorter for s:" << s
                      << " AttrName:" << AttrName
                      << endl;
            ret = contextset_less_cis_string<fh_context>( AttrName );
        }

        if( reverse )
        {
            ret = contextset_reverse<fh_context>(
                ContextSetCompare_FunctorData(""),
                ret );
        }

        if( lazy )
        {
            /*
             * Tack on a lazy sorting comparison function to the head
             * of the list so that existing and created contexts are
             * seperated before the rest of the sorting.
             */
            Fun_t lazyf =
                contextset_less_created_or_existed(
                    ContextSetCompare_FunctorData("") );

            ret = contextset_cascade<fh_context>(
                ContextSetCompare_FunctorData(""),
                lazyf,
                ret );
        }
        
        return ret;
    }

    /**
     * Allow many string pointers to share the same underlying byte contents.
     */
    static const char*
    makeStaticString( const std::string& s )
    {
        static stringset_t cache;
        stringset_t::iterator ci = cache.find( s );
        if( ci != cache.end() )
            return ci->c_str();

        cache.insert( s );
        ci = cache.find( s );
        return ci->c_str();
    }
    
    
    void
    ContextSetCompare::setup( const std::string& s )
    {
        d->m_sortString = 0;
        
//        cerr << "ContextSetCompare::setup() s:" << s << endl;
        
        if( s == "name" )
        {
            d->fun = contextset_rdn();
            return;
        }
        d->m_sortString = makeStaticString( s );
        
        typedef ContextSetComparePData::Fun_t Fun_t;
        int b = s.find("(");

        /*
         * Setup d->fun based on either a simple sorting predicate
         * or a chained predicate.
         */
        if( b == string::npos )
        {
            d->fun = createSortFunction( s );
        }
        else
        {
            string sortstring = s;

//            cerr << "Making a chained sorter for sortstring:" << sortstring << endl;
            
            /* Create chained sorting predicate */
            for( int i=0; string::npos != ( b = sortstring.find("(") ); ++i )
            {
                /* Skip the '(' delimiter */
                ++b;
                if( b > sortstring.length() )
                {
                    stringstream ss;
                    ss << "'(' char with no data after it"
                       << " string:" << s;
                    Throw_InvalidSortSpecification( tostr(ss), 0 );
                }

                /* Find the end of this part of the sort spec */
                int e = sortstring.find(")");
                if( e == string::npos || e < 1 )
                {
                    stringstream ss;
                    ss << "Unclosed meta info area (no second ')' found)"
                       << " string:" << s;
                    Throw_InvalidSortSpecification( tostr(ss), 0 );
                }
                
//                 cerr << "Found () match b:" << b << " e:" << e << " e-b:" << (e-b)
//                      << " original string:" << sortstring
//                      << endl;

                string partialSort = sortstring.substr( b, e-b );
                sortstring         = sortstring.substr( e+1 );
//                 PrePostTrimmer trimmer;
//                 trimmer.push_back( "(" );
//                 trimmer.push_back( ")" );
//                 partialSort = trimmer( partialSort );

//                 cerr << "Partial:" << partialSort
//                      << " rest of sortstring:" << sortstring
//                      << endl;
                
                Fun_t f = createSortFunction( partialSort );

                /* Either set d->fun for the first time or chain the new functor
                 * at the end of a cascade chain consisting of all the other
                 * functors
                 */
                if( !i )
                {
                    d->fun = f;
                }
                else
                {
                    d->fun = contextset_cascade<fh_context>(
                        ContextSetCompare_FunctorData(""), d->fun, f );
                }
            }
        }
        
        /*
         * Now we need to create a functor that has the user specified
         * sorting predicate first, and we tack a name based sort on
         * the end to ensure that sorting produces a total order.
         */
        d->fun = contextset_cascade<fh_context>(
            ContextSetCompare_FunctorData(""),
            d->fun,
            contextset_less_string<fh_context>( string("name") ) );
    }
    
    bool
    ContextSetCompare::operator()( const fh_context& s1, const fh_context& s2 ) const
    {
        return d->fun( s1, s2 ); 
    }
    bool
    ContextSetCompare::operator()( const fh_context& s1,
                                   const std::string& name ) const
    {
        return s1->getDirName() < name;
    }
    
    

    bool operator==( const ContextSetCompare& a, const ContextSetCompare& b )
    {
        if( !a.d->m_sortString && !b.d->m_sortString )
            return true;

        const char* astr = a.d->m_sortString ? a.d->m_sortString : "name";
        const char* bstr = b.d->m_sortString ? b.d->m_sortString : "name";
        if( !strcmp( astr, "" )) astr = "name";
        if( !strcmp( bstr, "" )) bstr = "name";
//        cerr << "csc operator==() a:" << astr << " b:" << bstr << endl;
        return !strcmp( astr, bstr );
    }
    bool operator!=( const ContextSetCompare& a, const ContextSetCompare& b )
    {
        return !( a==b );
    }
    
    
    namespace Factory
    {
        string ReverseSortStringOrder( string s )
        {
            if( s[0] != ':' )
            {
                string tmp = ":!:";
                tmp += s;
                return tmp;
            }
            else if( s.length() > 1 )
            {
                if( s[1] == '!' )
                {
                    string tmp = ":";
                    tmp += s.substr(2);
                    return tmp;
                }
                else
                {
                    string tmp = ":!";
                    tmp += s.substr(1);
                    return tmp;
                }
            }
            return s;
        }
        
        fh_context MakeSortedContext( fh_context& ctx, const std::string& s )
        {
            SortedContext* c = new SortedContext( ctx, s );
            fh_context ret;
            Upcast( ret, c );

            /* SortedContext assumes it is twice referenced by its parent */
            ret->AddRef();

            c->setIsChainedViewContextRoot();

//             cerr << "MakeSortedContext() ctx:" << ctx->getURL()
//                  << " s:" << s
//                  << " ret:" << ret->getURL()
//                  << endl;
            
            return ret;
        }
        
        fh_context MakeSortedContext( fh_context& ctx, fh_sorter& s )
        {
            SortedContext* c = new SortedContext( ctx, s->getString() );

//             DEBUG_dumpcl("MakeSortedContext(1)");

            fh_context ret = 0;
            Upcast( ret, c );

//            DEBUG_dumpcl("MakeSortedContext(2)");

            /* SortedContext assumes it is twice referenced by its parent */
            ret->AddRef();

//             DEBUG_dumpcl("MakeSortedContext(3)");
            
            c->setIsChainedViewContextRoot();
//             DEBUG_dumpcl("MakeSortedContext(end)");

//             cerr << "MakeSortedContext() ctx:" << ctx->getURL()
//                  << " s:" << s->getString()
//                  << " ret:" << ret->getURL()
//                  << endl;
            
            return ret;
        }

        fh_sorter MakeSorter( const std::string& s )
        {
            return new PreprocessedSortString( s );
        }
    };
};

