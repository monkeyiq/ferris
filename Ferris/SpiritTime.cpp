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

    $Id: SpiritTime.cpp,v 1.9 2010/09/24 21:31:00 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>


#define COMPILING_LIBFERRIS_SPIRIT_TIME_CAN_CRASH_SOME_CPUS_WORKAROUND

#include <General.hh>
#include <Ferris.hh>
#include <Resolver.hh>
#include <Enamel.hh>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <config.h>

// #ifdef GCC_HASCLASSVISIBILITY
// #pragma GCC visibility push(hidden)
// #endif

#define BOOST_SPIRIT_RULE_SCANNERTYPE_LIMIT 3

#include <boost/spirit.hpp>
using namespace boost::spirit;

#include <boost/function.hpp>
#include <boost/lambda/lambda.hpp>
using namespace boost::lambda;

// #ifdef GCC_HASCLASSVISIBILITY
// #pragma GCC visibility pop
// #endif

/**********/
/**********/
/**********/

using namespace std;

namespace Ferris
{
    namespace Time
    {
        struct FERRISEXP_DLLLOCAL ParseRelativeTimeTramp
        {
            typedef Loki::Functor<
                void, LOKI_TYPELIST_2( const char*, const char* ) >
            SAction_t;
            mutable SAction_t SAction;
            typedef const char* IteratorT;
    
            ParseRelativeTimeTramp( const SAction_t& SAction )
                :
                SAction( SAction )
                {
                }
            template <typename PointerToObj, typename PointerToMemFn>
            ParseRelativeTimeTramp( const PointerToObj& pObj, PointerToMemFn pMemFn )
                :
                SAction( SAction_t( pObj, pMemFn ) )
                {
                }

            void operator()( IteratorT first, IteratorT last) const
                {
                    SAction( first, last );
                }
        };

        struct FERRISEXP_DLLLOCAL ParseRelativeTimeBoostFunction
        {
            typedef boost::function< void( const char*, const char*)> SAction_t;
            mutable SAction_t SAction;
            typedef const char* IteratorT;
    
            ParseRelativeTimeBoostFunction( const SAction_t& SAction )
                :
                SAction( SAction )
                {
                }
            template <typename PointerToObj, typename PointerToMemFn>
            ParseRelativeTimeBoostFunction( const PointerToObj& pObj, PointerToMemFn pMemFn )
                :
                SAction( SAction_t( pObj, pMemFn ) )
                {
                }

            void operator()( IteratorT first, IteratorT last) const
                {
                    SAction( first, last );
                }
        };
        
        template < class T >
        struct ParseRelativeTimeTrampNumeric
        {
            typedef Loki::Functor<
                void, LOKI_TYPELIST_1( T ) >
            SAction_t;
            mutable SAction_t SAction;
    
            ParseRelativeTimeTrampNumeric( const SAction_t& SAction )
                :
                SAction( SAction )
                {
                }
            template <typename PointerToObj, typename PointerToMemFn>
            ParseRelativeTimeTrampNumeric( const PointerToObj& pObj, PointerToMemFn pMemFn )
                :
                SAction( SAction_t( pObj, pMemFn ) )
                {
                }

            void operator()( T n ) const
                {
                    SAction( n );
                }
        };

        
        struct FERRISEXP_DLLLOCAL ParseRelativeTime
        {
            typedef ParseRelativeTime _Self;
            typedef ParseRelativeTimeTramp F_t;
            typedef ParseRelativeTimeTrampNumeric< long > NF_t;
            typedef ParseRelativeTimeBoostFunction BF;
    
            template < typename PointerToMemFn >
            F_t F( PointerToMemFn pMemFun )
                {
                    return F_t( this, pMemFun );
                }
            template < typename PointerToMemFn >
            NF_t NF( PointerToMemFn pMemFun )
                {
                    return NF_t( this, pMemFun );
                }

    
    
            mutable time_t m_ret;      //< Return value that is build duing parse
            time_t m_relv;             //< Relative value for use in time offsets
            mutable struct tm m_tm;    //< broken down time version of m_relv;
            mutable struct tm m_tunit; //< broken down time translation for relative shifts
            mutable struct tm m_abtm;  //< broken down time translation for absolute times
            mutable bool      m_abtm_mon_used; //< since tm_mon is zero based flag if tm_mon is used
            mutable bool      m_ab_end; //< used for end 2006 type style
            mutable bool      m_yearDontClearDHMS; //< when parsing year don't clear Day, H, M and S.
            bool m_abtm_reset_smh;     //< zero the sec, min, hour of the absolute time
            long m_mag;                //< number of units to shift m_tunit
            long m_dir;                //< direction (future/past) for the application of m_tunit
            bool m_blast;              //< begining of last block
            bool m_elast;              //< ending of last block
            bool m_bnext;              //< begining of next block
            bool m_enext;              //< ending of next block
            bool m_bthis;              //< begining of this block
            bool m_ethis;              //< ending of this block
            string m_func_URL;         //< for calling getStrAttr()
            string m_func_eaname;      //< for calling getStrAttr()
    
            void fn_yesterday( const char* str, const char* end )
                {
                    m_tm.tm_sec = 0;
                    m_tm.tm_min = 0;
                    m_tm.tm_hour = 0;
                    m_tm.tm_mday--;
                    m_ret = mktime( &m_tm );
                }

            void fn_tomorrow( const char* str, const char* end )
                {
                    m_tm.tm_sec = 0;
                    m_tm.tm_min = 0;
                    m_tm.tm_hour = 0;
                    m_tm.tm_mday++;
                    m_ret = mktime( &m_tm );
                }

            void fn_today( const char* str, const char* end )
                {
                    m_tm.tm_sec = 0;
                    m_tm.tm_min = 0;
                    m_tm.tm_hour = 0;
                    m_ret = mktime( &m_tm );
                }
            void fn_namedday(  const char* str, const char* end )
                {
                    m_ret = mktime( &m_tm );
                }
    
            void fn_setDirection( const char* str, const char* end )
                {
                    string s( str, end );

                    if( contains( str, "last" ) ) m_dir = -1;
                    else                          m_dir = 1;
            
                }
    
            void fn_setMagnitude( long n )
                {
                    LG_RTIMEPARSE_D << "fn_setMagnitude() n:" << n << endl;
                    LG_RTIMEPARSE_D << "fh_setMagnitude m_tunit:" << tostr(m_tunit) << endl;
                    LG_RTIMEPARSE_D << "fh_setMagnitude m_tm:" << tostr(m_tm) << endl;
                    m_mag = n;
                }

            void reset()
                {
                    m_func_URL = "";
                    m_func_eaname = "";
                    m_mag = 1;
                    m_dir = 1;
                    bzero( &m_tunit, sizeof(m_tunit));
                    bzero( &m_abtm,  sizeof(m_abtm));
                    m_abtm_reset_smh = false;
                    m_blast = false;
                    m_elast = false;
                    m_bnext = false;
                    m_enext = false;
                    m_bthis = false;
                    m_ethis = false;
                    m_abtm_mon_used = false;
                    m_ab_end = false;
                    m_yearDontClearDHMS = false;
                }
    
            void fn_updateRetWithDirMagTUnit( const char* str, const char* end )
                {
                    LG_RTIMEPARSE_D << "m_tunit:" << endl
                                    << tostr(m_tunit) <<  endl;
                    LG_RTIMEPARSE_D << "m_tm:" << endl
                                    << tostr(m_tm) <<  endl;
            
                    m_tm = m_tm + m_tunit;

                    LG_RTIMEPARSE_D << "m_tm-result:" << endl
                                    << tostr(m_tm) <<  endl;
                    m_ret = mktime( &m_tm );

                    reset();
                }

            int choose( int a, int b )
                {
                    return a ? a : b;
                }
    
            void fn_updateRetReplaceWithNonZeroAbTm( const char* str, const char* end )
                {
                    LG_RTIMEPARSE_D << "fn_updateRetReplaceWithNonZeroTUnit()" << endl;
                    LG_RTIMEPARSE_D << "m_abtm:" << endl
                                    << tostr(m_abtm) <<  endl;
                    LG_RTIMEPARSE_D << "m_tm:" << endl
                                    << tostr(m_tm) <<  endl;
            
                    m_tm.tm_year = choose( m_abtm.tm_year, m_tm.tm_year );
                    m_tm.tm_mon  = choose( m_abtm.tm_mon,  m_tm.tm_mon  );
                    if( m_abtm_mon_used )
                        m_tm.tm_mon = m_abtm.tm_mon;
                    m_tm.tm_mday = choose( m_abtm.tm_mday, m_tm.tm_mday );
                    m_tm.tm_hour = choose( m_abtm.tm_hour, m_tm.tm_hour );
                    m_tm.tm_min  = choose( m_abtm.tm_min,  m_tm.tm_min  );
                    m_tm.tm_sec  = choose( m_abtm.tm_sec,  m_tm.tm_sec  );

                    if( m_abtm_reset_smh )
                    {
                        m_tm.tm_hour = 0;
                        m_tm.tm_min  = 0;
                        m_tm.tm_sec  = 0;
                    }
            
                    LG_RTIMEPARSE_D << "m_tm-result:" << endl
                                    << tostr(m_tm) <<  endl;
            
                    m_ret = mktime( &m_tm );
                    reset();
                }
    
    
            bool begend_inuse()
                {
                    return m_blast || m_elast || m_bnext || m_enext
                        || m_bthis || m_ethis;
                }
            bool end_inuse()
                {
                    return m_elast || m_enext || m_ethis;
                }
    
            void fn_begend_sec( const char* str, const char* end )
                {
                    if( m_blast || m_bnext || m_bthis )
                        m_tm.tm_sec = 0;
                    if( m_elast || m_enext || m_ethis )
                        m_tm.tm_sec = 59;
                    if( m_bthis || m_ethis )
                        m_tm.tm_min--;
                }
            void fn_begend_min( const char* str, const char* end )
                {
                    if( begend_inuse() )
                    {
                        m_tm.tm_sec = 0;
                        m_tm.tm_min = 0;
                    }
                    if( m_elast || m_enext )
                    {
                        m_tm.tm_hour++;
                    }
                    if( m_bthis )
                        m_tm.tm_hour--;

                    if( end_inuse() )
                        m_tm.tm_sec--;
                }
            void fn_begend_hour( const char* str, const char* end )
                {
                    if( begend_inuse() )
                    {
                        m_tm.tm_sec = 0;
                        m_tm.tm_min = 0;
                        m_tm.tm_hour = 0;
                    }
                    if( m_elast || m_enext )
                    {
                        m_tm.tm_mday++;
                    }
                    if( m_bthis )
                        m_tm.tm_mday--;
                    
                    if( end_inuse() )
                        m_tm.tm_sec--;
                }
            void fn_begend_week( const char* str, const char* end )
                {
                    if( begend_inuse() )
                    {
                        m_tm.tm_sec = 0;
                        m_tm.tm_min = 0;
                        m_tm.tm_hour = 0;
                    }
                    if( m_blast || m_elast )
                    {
                        m_tm.tm_mday = m_tm.tm_mday - m_tm.tm_wday + 1;
                        if( !m_tm.tm_wday )
                            m_tm.tm_mday -= 7;
                        if( m_elast )
                            m_tm.tm_mday += 7;
                    }
                        
                    if( m_bnext )
                    {
                        m_tm.tm_mday -= m_tm.tm_wday;
                        ++m_tm.tm_mday;
                    }
                    if( m_enext )
                    {
                        m_tm.tm_mday -= m_tm.tm_wday;
                        m_tm.tm_mday += 8;
                    }
                    if( m_bthis )
                    {
                        m_tm.tm_mday -= m_tm.tm_wday;
                        m_tm.tm_mday -= 6;
                    }
                    if( m_ethis )
                    {
                        m_tm.tm_mday -= m_tm.tm_wday;
                        m_tm.tm_mday += 1;
                    }
                    if( end_inuse() )
                        m_tm.tm_sec--;
                }

            void fn_begend_q( const char* str, const char* end )
                {
                    if( begend_inuse() )
                    {
                        m_tm.tm_sec = 0;
                        m_tm.tm_min = 0;
                        m_tm.tm_hour = 0;
                    }
                    if( begend_inuse() )
                        m_tm.tm_mon = m_tm.tm_mon - (m_tm.tm_mon%3);
                    if( m_elast || m_enext || m_ethis )
                        m_tm.tm_mon += 3;
                    if( end_inuse() )
                        m_tm.tm_sec--;
                }
            void fn_begend_mday( const char* str, const char* end )
                {
                    if( begend_inuse() )
                    {
                        m_tm.tm_sec = 0;
                        m_tm.tm_min = 0;
                        m_tm.tm_hour = 0;
                    }

                    if( begend_inuse() )
                        m_tm.tm_mday = 1;
                    if( m_elast || m_enext )
                        m_tm.tm_mon++;
                    if( m_bthis )
                        m_tm.tm_mon--;
                    if( end_inuse() )
                        m_tm.tm_sec--;
                }
            void fn_begend_month( const char* str, const char* end )
                {
                    if( begend_inuse() )
                    {
                        m_tm.tm_sec = 0;
                        m_tm.tm_min = 0;
                        m_tm.tm_hour = 0;
                        m_tm.tm_mday = 1;
                        m_tm.tm_mon = 0;
                    }

                    if( m_bthis || m_ethis )
                        m_tm.tm_year--;
                    
                    if( m_elast || m_enext || m_ethis )
                    {
                        m_tm.tm_year++;
                    }
                    else if( begend_inuse() )
                    {
                        m_tm.tm_mday = 1;
                    }
                    
                    if( end_inuse() )
                    {
                        m_tm.tm_sec--;
                    }
                }

            void fn_ago( const char* str, const char* end )
                {
                    int v = -1;

                    LG_RTIMEPARSE_D << "fh_ago m_tunit:" << tostr(m_tunit) << endl;
            
                    m_tunit.tm_year *= v;
                    m_tunit.tm_mon  *= v;
                    m_tunit.tm_mday *= v;
                    m_tunit.tm_hour *= v;
                    m_tunit.tm_min  *= v;
                    m_tunit.tm_sec  *= v;
                }

            void fn_tunit_update_month_trim_mday( int mod )
                {
                    int re = m_tm.tm_mday%mod;
                    if( m_tm.tm_mday > mod )
                        m_tm.tm_mday -= re;
                }
    
            void fn_tunit_update_month( const char* str, const char* end )
                {
                    /*
                      0  J 31
                      1  F 28
                      2  M 31
                      3  A 30
                      4  M 31
                      5  J 30
                      6  J 31
                      7  A 31
                      8  S 30
                      9  O 31
                      0  N 30
                      1  D 31
                    */          
            
                    m_tunit.tm_mon = m_mag * m_dir;
                    int newmonth = (m_tm.tm_mon + m_tunit.tm_mon) % 12;
                    LG_RTIMEPARSE_D << "newmonth:" << newmonth << endl;

                    std::set<int> daysOf30;
                    daysOf30.insert( 3 );
                    daysOf30.insert( 5 );
                    daysOf30.insert( 8 );
                    daysOf30.insert( 10 );
            
                    if( newmonth == 1 )
                    {
                        if( !(m_tm.tm_year % 4 ))
                            fn_tunit_update_month_trim_mday( 29 );
                        else
                            fn_tunit_update_month_trim_mday( 28 );
                    }
                    if( daysOf30.end() != daysOf30.find( newmonth ) )
                    {
                        fn_tunit_update_month_trim_mday( 30 );
                    }

                }

            /********/
            /********/
            /********/

            void ab_sec( int n )
                {
                    m_abtm.tm_sec = n;
                }
            void ab_min( int n )
                {
                    m_abtm.tm_min = n;
                    m_abtm.tm_sec = 0;
                }
            void ab_hr( int n )
                {
                    m_abtm.tm_hour = n;
                    m_abtm.tm_min = 0;
                    m_abtm.tm_sec = 0;
                }
            void ab_day( int n )
                {
                    m_abtm.tm_sec = 0;
                    m_abtm.tm_min = 0;
                    m_abtm.tm_hour = 0;
                    m_abtm.tm_mday = n;
                }
            void ab_month( int n )
                {
                    m_abtm.tm_sec = 0;
                    m_abtm.tm_min = 0;
                    m_abtm.tm_hour = 0;
                    m_abtm.tm_mday = 1;
                    m_abtm.tm_mon = n - 1;
                    m_abtm_mon_used = true;
                }
            void ab_year( int n )
                {
                    if( n < 100 )
                        n += 100;
                    if( n > 1900 )
                        n -= 1900;

                    if( !m_yearDontClearDHMS )
                    {
                        m_abtm.tm_sec = 0;
                        m_abtm.tm_min = 0;
                        m_abtm.tm_hour = 0;
                        m_abtm.tm_mday = 1;
                        m_abtm.tm_mon = 0;
                        m_abtm_mon_used = true;
                    }
                    
                    m_abtm.tm_year = n;

                    if( m_ab_end )
                    {
                        m_abtm.tm_year++;
                        fn_updateRetReplaceWithNonZeroAbTm( 0, 0 );
                        m_abtm.tm_sec--;
                    }
                }
    
            /********/
            /********/
            /********/

    
            void fn_func_url( const char* beg, const char* end )
                {
                    string str( beg, end );
                    m_func_URL = str;
                }

            void fn_func_eaname( const char* beg, const char* end )
                {
                    string str( beg, end );
                    m_func_eaname = str;
                }
    
            void fn_func_perform( const char* beg, const char* end )
                {
                    if( m_func_eaname.empty() || m_func_URL.empty() )
                    {
                        LG_RTIMEPARSE_W
                            << "Error, must have both URL and EA to read for absolute time" << endl;
                        cerr
                            << "Error, must have both URL and EA to read for absolute time" << endl;
                        reset();
                        return;
                    }

                    fh_context c = Resolve( m_func_URL );
                    string s = getStrAttr( c, m_func_eaname, "", true, true );

                    time_t tt = toType<time_t>( s );
            
                    m_relv = tt;
                    m_ret  = tt;
                    m_tm = *(localtime( &tt ));
                    reset();
                }
    
    

            /********/
            /********/
            /********/

#ifdef PLATFORM_OSX
            time_t operator()( const std::string& s, time_t relv  )
                {
                    cerr << "WARN: SpiritTime is not working on osx yet" << endl;
                    return 0;
                }
#else  
            time_t operator()( const std::string& s, time_t relv  )
                {
#ifdef GCC_HAS_BUGS_WITH_SPIRIT
                    return 0;
#else

                    // yay, if multi_index is included anywhere it means
                    // we can't just use lambda natively.
#define _1 boost::lambda::_1 
#define _2 boost::lambda::_2 
                    
                    m_relv = relv;
                    m_ret  = relv;
                    m_tm = *(localtime( &relv ));
                    reset();

                    
                    typedef scanner_list<scanner<>, phrase_scanner_t> scanners;
                    typedef rule< scanners > R;

// update the field 'v' in the m_tunit using an optional 'm'
// magnitude to multiply the update value by
// (eg, quarterly = 3*monthly )
#define TUUPD_MAG( v, m )                                               \
                    [ BF((_1, var(m_tunit.v) = m * var(m_mag) * var(m_dir) )) ] >> ops
#define TUUPD( v ) TUUPD_MAG( v, 1 )

#define NAMED_DAY( offsetFromMonday )                                   \
                    [ BF(( _1,                                          \
                           var(m_tm.tm_sec) = 0,                        \
                           var(m_tm.tm_min) = 0,                        \
                           var(m_tm.tm_hour) = 0,                       \
                           var(m_tm.tm_mday) = var(m_tm.tm_mday) - var(m_tm.tm_wday) + offsetFromMonday + 1 )) ] \
                        [ F( &_Self::fn_namedday ) ]

                    R l_monday    = (str_p("monday")    | str_p("mon"));
                    R l_tuesday   = (str_p("tuesday")   | str_p("tue"));
                    R l_wednesday = (str_p("wednesday") | str_p("wed"));
                    R l_thursday  = (str_p("thursday")  | str_p("thu"));
                    R l_friday    = (str_p("friday")    | str_p("fri"));
                    R l_saturday  = (str_p("saturday")  | str_p("sat"));
                    R l_sunday    = (str_p("sunday")    | str_p("sun"));
            
                    R minus     = ch_p('-');
                    R plus      = ch_p('+');
                    R ago       = str_p("ago");
                    R ops       = !(ch_p('s'));
                    R yesterday = str_p("yesterday")  | str_p("yes") | str_p("yester");
                    R tomorrow  = str_p("tomorrow")   | str_p("tom") | str_p("tomor");
                    R today     = str_p("today")      | str_p("tod") | str_p("td");
                    R monday    = (l_monday)NAMED_DAY(0);
                    R tuesday   = (l_tuesday)NAMED_DAY(1);
                    R wednesday = (l_wednesday)NAMED_DAY(2);
                    R thursday  = (l_thursday)NAMED_DAY(3);
                    R friday    = (l_friday)NAMED_DAY(4);
                    R saturday  = (l_saturday)NAMED_DAY(5);
                    R sunday    = (l_sunday)NAMED_DAY(6);
                    R AllDays   = monday | tuesday | wednesday | thursday | friday | saturday | sunday;

                    R last      = str_p("last");
                    R next      = str_p("next");
                    R blast     = (str_p("blast")     | str_p("begin last") | str_p("begining last")
                                   | str_p("beginning last") | str_p("beginning of last") )
                        [ BF(( _1, var(m_blast)=true )) ];
                    R elast     = (str_p("elast")     | str_p("end last") | str_p("ending last")
                                   | str_p("ending of last") )
                        [ BF(( _1, var(m_elast)=true )) ];
                    R bnext     = (str_p("bnext")     | str_p("begin next") | str_p("begining next")
                                   | str_p("beginning next") | str_p("beginning of next") )
                        [ BF(( _1, var(m_bnext)=true )) ];
                    R enext     = (str_p("enext")     | str_p("end next") | str_p("ending next")
                                   | str_p("ending of next") )
                        [ BF(( _1, var(m_enext)=true )) ];
                    R bthis     = (str_p("bthis")     | str_p("begin this") | str_p("begining this")
                                   | str_p("beginning this") | str_p("beginning of this") )
                        [ BF(( _1, var(m_bthis)=true )) ];
                    R ethis     = (str_p("ethis")     | str_p("end this") | str_p("ending this")
                                   | str_p("ending of this") )
                        [ BF(( _1, var(m_ethis)=true )) ];
                    R Minute = (str_p("minute")  | str_p("min"))
                        [ F(&_Self::fn_begend_sec) ]
                        TUUPD(tm_min);
                    R Hour   = (str_p("hour")    | str_p("hr") | str_p("h"))
                        [ F(&_Self::fn_begend_min) ]
                        TUUPD(tm_hour);
                    R Day    = (str_p("day")     | str_p("d"))
                        [ F(&_Self::fn_begend_hour) ]
                        TUUPD(tm_mday);
                    R Week   = (str_p("week")    | str_p("wk") | str_p("w"))
                        [ F(&_Self::fn_begend_week) ]
                        TUUPD_MAG(tm_mday,7);
                    R Month  = (str_p("month") | str_p("mon") | str_p("mo"))
                        [ F(&_Self::fn_begend_mday) ]
                        [ F(&_Self::fn_tunit_update_month ) ]
                        >> ops;
                    R Year   = (str_p("year")    | str_p("yr") | str_p("y"))
                        [ F(&_Self::fn_begend_month) ]
                        TUUPD(tm_year);
                    R Quar   = (str_p("quarter") | str_p("qu"))
                        TUUPD_MAG( tm_mon, 3 )
                        [ F(&_Self::fn_begend_q) ];
                    R TimeUnit  = ( Minute | Hour | Minute | Day | Week | Month | Quar | Year );


                    /****************************************/
                    /****************************************/
                    /****************************************/
            
#define AB_NAMED_MON( month )                               \
                    [ BF(( _1,                              \
                           var( m_abtm_reset_smh ) = true,  \
                           var(m_abtm.tm_mday) = 1,         \
                           var(m_abtm_mon_used) = true,     \
                           var(m_abtm.tm_mon) = month )) ]
#define AB_NAMED_DAY( offsetFromMonday )                                \
                    [ BF(( _1,                                          \
                           var( m_abtm_reset_smh ) = true,              \
                           var(m_abtm.tm_mday) = var(m_tm.tm_mday) - var(m_tm.tm_wday) + offsetFromMonday + 1 )) ]
            
                    R ab_bropen  = ch_p('(');
                    R ab_brclose = ch_p(')');
                    R ab_squote = ch_p('\'');
                    R ab_dquote = ch_p('\"');
                    R ab_comma = ch_p(',');
                    R ab_colon = ch_p(':');
                    R ab_dash  = ch_p('-');
                    R ab_slash = ch_p('/');
                    R ab_space = ch_p(' ');
                    R ab_sec = uint_parser<int, 10, 1, 2 >()[ NF( &_Self::ab_sec) ];
                    R ab_min = uint_parser<int, 10, 1, 2 >()[ NF( &_Self::ab_min) ];
                    R ab_hr  = uint_parser<int, 10, 1, 2 >()[ NF( &_Self::ab_hr)  ];
                    R ab_time =
                        ( ab_hr
                          >> ab_colon >> ab_min
                          >> !(ab_colon >> ab_sec)
                            );

                    R ab_datesep   = ab_dash | ab_slash;
                    R ab_year     = (ch_p('0') >> uint_parser<int, 10, 1, 1 >()[ NF( &_Self::ab_year) ])
                        | uint_parser<int, 10, 4, 4 >()[ NF( &_Self::ab_year) ];
                    R ab_jan      = (str_p("january")  | str_p("jan") | str_p("ja"))AB_NAMED_MON( 0 );
                    R ab_feb      = (str_p("feburary") | str_p("feb") | str_p("fe"))AB_NAMED_MON( 1 );
                    R ab_mar      = (str_p("march")    | str_p("mar") | str_p("ma"))AB_NAMED_MON( 2 );
                    R ab_apr      = (str_p("april")    | str_p("apr") | str_p("ap"))AB_NAMED_MON( 3 );
                    R ab_may      = (str_p("may")      | str_p("ma")               )AB_NAMED_MON( 4 );
                    R ab_jun      = (str_p("june")     | str_p("jun") | str_p("jn"))AB_NAMED_MON( 5 );
                    R ab_jul      = (str_p("july")     | str_p("jul") | str_p("jl"))AB_NAMED_MON( 6 );
                    R ab_aug      = (str_p("august")   | str_p("aug") | str_p("au"))AB_NAMED_MON( 7 );
                    R ab_sep      = (str_p("september")| str_p("sep") | str_p("se"))AB_NAMED_MON( 8 );
                    R ab_oct      = (str_p("october")  | str_p("oct") | str_p("oc"))AB_NAMED_MON( 9 );
                    R ab_nov      = (str_p("november") | str_p("nov") | str_p("no"))AB_NAMED_MON( 10 );
                    R ab_dec      = (str_p("december") | str_p("dec") | str_p("de"))AB_NAMED_MON( 11 );
                    R ab_monthStr = ( ab_jan | ab_feb | ab_mar | ab_apr | ab_may | ab_jun
                                      | ab_jul | ab_aug | ab_sep | ab_oct | ab_nov | ab_dec );
                    R ab_monthNum = uint_parser<int, 10, 1, 2 >()[ NF( &_Self::ab_month) ];
                    R ab_month    = ( ab_monthNum | ab_monthStr );
                    R ab_monday    = (l_monday)AB_NAMED_DAY(0);
                    R ab_tuesday   = (l_tuesday)AB_NAMED_DAY(1);
                    R ab_wednesday = (l_wednesday)AB_NAMED_DAY(2);
                    R ab_thursday  = (l_thursday)AB_NAMED_DAY(3);
                    R ab_friday    = (l_friday)AB_NAMED_DAY(4);
                    R ab_saturday  = (l_saturday)AB_NAMED_DAY(5);
                    R ab_sunday    = (l_sunday)AB_NAMED_DAY(6);
                    R ab_dayStr   =  ab_monday | ab_tuesday | ab_wednesday
                        | ab_thursday | ab_friday | ab_saturday | ab_sunday;
                    R ab_dayNum   = uint_parser<int, 10, 1, 2 >()[ NF( &_Self::ab_day) ];
                    R ab_day      = ( ab_dayNum | ab_dayStr );

                    R ab_begin     = str_p("begin")
                        [ BF(( _1, var( m_abtm_reset_smh ) = true )) ];
                    R ab_end       = str_p("end")
                        [ BF(( _1, var( m_abtm_reset_smh ) = true, var(m_ab_end)=true )) ];

                    R ab_ctimeString =
                        (ab_dayStr >> ab_month >> ab_dayNum)[ BF(( _1, var( m_abtm_reset_smh ) = false ))]
                        >> ab_time[ BF(( _1, var( m_yearDontClearDHMS ) = true )) ]
                        >> ab_year;
                    
                    R ab_dateHeur = 
                        (
                            ( ab_ctimeString
                              | ab_begin >> ab_year
                              | ab_end >> ab_year
                              | !(ab_year >> !(ab_datesep))
                              >> (
                                  (ab_monthNum >> ab_datesep >> ab_day)
                                  |
                                  (ab_monthStr >> !(!(ab_datesep) >> ab_day))
                                  |
                                  (ab_day >> ab_datesep >> ab_month)
                                  )
                                )
                            );

                    //
                    // Allow calling getea( url, eaname ) and mtime( url )
                    // for setting an absolute time
                    //
                    R ab_notsqstr = (+( ~ch_p('\'') ));
                    R ab_notdqstr = (+( ~ch_p('\"') ));
                    R ab_notcommastr = (+( ~ch_p(',') ));
                    R ab_notclbrstr  = (+( ~ch_p(')') ));
                    R ab_getEAf  = str_p("getea");
                    R ab_getEAmt = str_p("mtime");
                    R ab_getEAat = str_p("atime");
                    R ab_quotedURL =
                        ( ab_squote >> ab_notsqstr[ F(&_Self::fn_func_url) ] >> ab_squote )
                        |
                        ( ab_dquote >> ab_notdqstr[ F(&_Self::fn_func_url) ] >> ab_dquote )
                        ;
                    R ab_quotedEAName =
                        ( ab_squote >> ab_notsqstr[ F(&_Self::fn_func_eaname) ] >> ab_squote )
                        |
                        ( ab_dquote >> ab_notdqstr[ F(&_Self::fn_func_eaname) ] >> ab_dquote )
                        ;
                    R ab_dateEA = (
                        (
                            ab_getEAf >> ab_bropen
                            >> (
                                ab_quotedURL
                                |
                                ab_notcommastr[ F(&_Self::fn_func_url) ]
                                )
                            >> ab_comma
                            >> (
                                ab_quotedEAName
                                |
                                ab_notclbrstr[ F(&_Self::fn_func_eaname) ]
                                )
                            >> ab_brclose
                            )[ F( &_Self::fn_func_perform ) ]
                        |
                        (
                            ab_getEAf >> ab_quotedURL >> !(ab_comma) >> ab_quotedEAName
                            )[ F( &_Self::fn_func_perform ) ]
                        |
                        (
                            ( ab_getEAmt | ab_getEAat )[ F(&_Self::fn_func_eaname) ]
                            >> ab_bropen >>
                            (
                                ab_quotedURL
                                |
                                ab_notclbrstr[ F(&_Self::fn_func_url) ]
                                )
                            >> ab_brclose
                            )[ F( &_Self::fn_func_perform ) ]
                        |
                        (
                            ( ab_getEAmt | ab_getEAat )[ F(&_Self::fn_func_eaname) ]
                            >> ab_quotedURL
                            )[ F( &_Self::fn_func_perform ) ]
                         )
                        ;
            
                    R ab_date =
                        ab_dateHeur | ab_dateEA
                        ;
            

                    R CustomTimeStamp = (
                        ( ab_date >> ab_time )
                        | ab_date
                        | ab_time
                        )
                        [ F( &_Self::fn_updateRetReplaceWithNonZeroAbTm ) ];

                    /****************************************/
                    /****************************************/
                    /****************************************/
            
                    R PlusMinusRelativeTime = +(
                        !( minus[ BF((_1, var(m_dir)=-1 )) ]
                           | plus[ BF((_1, var(m_dir)=1 )) ]
                            )
                        >> uint_p[ NF( &_Self::fn_setMagnitude) ]
                        >> TimeUnit
                        >> !(
                            ago[ F( &_Self::fn_ago ) ]
                            )
                        )[ F( &_Self::fn_updateRetWithDirMagTUnit ) ];

                    R relativeTime_last_week_p =
                        +(
                                (last|next)[ F( &_Self::fn_setDirection ) ]
                                >> !(uint_p[ NF( &_Self::fn_setMagnitude) ])
                                >> TimeUnit
                                )[ F( &_Self::fn_updateRetWithDirMagTUnit ) ]
                            >> !(PlusMinusRelativeTime);
                    R relativeTime_last_year_p =
                        +(
                                (blast|elast|bnext|enext|bthis|ethis)[ F( &_Self::fn_setDirection ) ]
                                >> !(uint_p[ NF( &_Self::fn_setMagnitude) ])
                                >> TimeUnit
                                )[ F( &_Self::fn_updateRetWithDirMagTUnit ) ]
                            >> !(PlusMinusRelativeTime);
                    R relativeTime_last_p = 
                        relativeTime_last_week_p
                        | relativeTime_last_year_p;
                    
                    R relativeTime_p
                        = yesterday [ F( &_Self::fn_yesterday ) ]
                        | tomorrow  [ F( &_Self::fn_tomorrow ) ]
                        | today     [ F( &_Self::fn_today ) ]
                        | AllDays
                        | relativeTime_last_p
                        // + 6 days - 3 months
                        | PlusMinusRelativeTime
                        ;

                    R relativeTimeStamp_p =
                        CustomTimeStamp >> !(relativeTime_p)
                        | relativeTime_p;

                    string lowers = tolowerstr()(s);
                    parse_info<> info = parse(
                        lowers.c_str(),
                        relativeTimeStamp_p,
                        space_p );
            
                    if (info.full)
                    {
                    }
                    else
                    {
                        fh_stringstream ss;
                        ss << "Parsing relative time string failed" << nl
                           << "input:" << s << nl
                           << "stopped at: \": " << info.stop << "\"" << nl
                           << "char offset:" << ( info.stop - s.c_str() ) << nl;
                        LG_RTIMEPARSE_W << tostr(ss) << endl;
                        cerr << tostr(ss) << endl;

                        BackTrace();
                        
                        Throw_RelativeTimeParsing( tostr(ss), 0 );
                    }
            
                    return m_ret;
#endif
                }
#endif
            
        };

        time_t ParseRelativeTimeString( const std::string& s, time_t relv  )
        {
            if( relv == 0 )
                relv = getTime();

            if( struct tm* tm = getdate( s.c_str() ) )
            {
                LG_TIME_D << "ParseRelativeTimeString() getdate() succeeded" << endl;
                time_t ret = mktime( tm );
                return ret;
            }
            LG_TIME_D << "ParseRelativeTimeString() getdate() failed. getdate_err:" << getdate_err << endl;
            
            ParseRelativeTime obj;
            return obj( s, relv );
        }
        
        
    };
};
