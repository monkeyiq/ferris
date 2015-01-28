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

    $Id: General.hh,v 1.17 2010/11/17 21:30:44 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#ifndef _ALREADY_INCLUDED_FERRIS_GENERAL_H_
#define _ALREADY_INCLUDED_FERRIS_GENERAL_H_

#include <Ferris/HiddenSymbolSupport.hh>

#include <glib.h>

#include <string>
#include <map>
#include <Ferris/FerrisSTL.hh>
#include <Ferris/TypeDecl.hh>
#include <FerrisStreams/Streams.hh>
#include <sstream>

#pragma GCC visibility push(default)

namespace Ferris
{
    FERRISEXP_API std::string remove_prefix( const std::string& ret,  const std::string& prefix );
    FERRISEXP_API std::string remove_postfix( const std::string& ret, const std::string& postfix );
    
    FERRISEXP_API std::string tostr( const struct tm& tm );
    FERRISEXP_API stringpair_t split( const std::string& data, const std::string& sep );
    
    /**
     * Convert s into upper case.
     */
    FERRISEXP_API std::string toupperstring( const std::string& s );
    FERRISEXP_API std::string tolowerstring( const std::string& s );

    /**
     * Convert a perhaps relative path to an absolute one.
     * The current working directory can be given explicitly in cwd.
     * Note that this handles URLs as being absolute.
     *
     * This is an attempt because the path itself is not checked for existance.
     */
    FERRISEXP_API std::string attemptToAbsoluteURL(
        const std::string& maybeRelative,
        const std::string& cwd = "" );
    
    /**
     * Put a narrow string encoded in UTF-8 into a wide io stream.
     * This function allows such wonderful things as
     * wcout << "foo" << std::string("some utf8 const") << endl;
     * to be written and basically removes the need to ever consider
     * using cout in an I18N application.
     */
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    operator<<(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >& oss, const std::string& s );

    FERRISEXP_API std::istream& getline( std::istream& ss, std::string& ret, const std::string& delim );

    FERRISEXP_API std::string chomp( const std::string& s );
    
    namespace Util
    {
        /**
         * Convert a wide string to a UTF-8 encoded string
         */
        FERRISEXP_API std::string& wstring_to_utf8( std::string& ret, const std::wstring& s );
        FERRISEXP_API std::string  wstring_to_utf8( const std::wstring& s );
        
        /**
         * Convert a long string from 's' into a wide string.
         * The string 's' is assumed to be in UTF-8 format and the 'ret'
         * string will contain wide characters.
         */
        FERRISEXP_API std::wstring& utf8_to_wstring( std::wstring& ret, const std::string& s );
        FERRISEXP_API std::wstring  utf8_to_wstring( const std::string& s );

        inline std::wstring toStreamChar( wchar_t ch ) 
        { return std::wstring(&ch,1) ; }
        inline char toStreamChar( char ch ) { return ch ; }
        

        /**
         * Convert an ISO 8859-1 string into UTF8
         */
        FERRISEXP_API std::string iso8859_to_utf8( const std::string& s );

        /**
         * Try to patch away encodings like iso8859 into UTF8 for a given URL
         */
        std::string convert_url_to_utf8( const std::string& earl );
        std::string convert_basename_to_utf8( const std::string& v );


        /**
         * Decode the %20 type strings in a URL encoded string and
         * replace with their real characters.
         */
        std::string URLDecode( const std::string& s );


        /**
         * Escape any special regex characters in the string to have no special meaning.
         */
        std::string EscapeStringAsRegex( const std::string&s );
        
        /**
         * replace all occurances of oldc with newc in a copy of s and return the
         * copy
         */
        FERRISEXP_API std::string replace_all( const std::string& s, char oldc, char newc );
        FERRISEXP_API std::string replace_all( const std::string& s, char oldc, const std::string& news );
        FERRISEXP_API std::string replace_all( const std::string& s,
                                               const std::string& olds,
                                               const std::string& news );

        /**
         * like replace_all() but only work on the first match instance
         */
        FERRISEXP_API std::string replace_first( const std::string& s,
                                                 const std::string& olds,
                                                 const std::string& news );
        
        
        /**
         * Convert a string like 10k, 200Mb, 34m, 6g to the integer value
         * of that size, eg. 10240 == convertByteString( "10k" );
         */
        FERRISEXP_API guint64 convertByteString( const std::string& s );

        /**
         * Create a human readable string showing the given size
         */
        FERRISEXP_API std::string convertByteString( guint64 v );

        /**
         * Convert an "extraData" string to a map.
         * Used mainly internally by decorator contexts
         *
         * Takes input of the form kvs="hi=there&key=value:more=data/"
         * where the trailing slash is optional and key/value pairs
         * are seperated by any char given in seps
         *
         * @param kvs The string to break apart
         * @param seps The seperators between each kv pair
         *
         * @returns A map of strings with each key and value from s
         * placed into it for lookup by key
         */
        FERRISEXP_API StringMap_t&
        ParseKeyValueString( StringMap_t& ret,
                             const std::string& kvs,
                             const std::string& seps = ":&" );
        FERRISEXP_API StringMap_t
        ParseKeyValueString( const std::string& kvs,
                             const std::string& seps = ":&" );
        FERRISEXP_API StringMap_t
        ParseKeyValueString( fh_istream& iss,
                             const std::string& seps = ":&" );
        /**
         * Create a string suitable for parsing using ParseKeyValueString()
         *
         */
        FERRISEXP_API std::string CreateKeyValueString( const StringMap_t& sm,
                                                        const std::string kvsep = "=",
                                                        const std::string atomsep = "&" );


        
        /**
         * convert the string 's' which is in the form item1,item2,item3
         * into a collection. This is much like the STL copy() algorithm
         * in that the ouput iterator should either point to a collection
         * that has been resized to allow all items to be written or to
         * a inserter wrapper for a raw iterator.
         *
         */
        template< class Col, class OutputIterator >
        OutputIterator parseSeperatedList( const std::string& s,
                                           Col& c,
                                           OutputIterator out,
                                           const char sepchar = ',' )
        {
            std::string tmp;

            std::stringstream ss(s);
            while( std::getline( ss, tmp, sepchar ))
                if( !tmp.empty() )
                {
                    typename Col::value_type r;
                    std::stringstream data;
                    data << tmp;
                    data >> std::noskipws >> r;

                    *++out = r;
                }

            return out;
        }
        /**
         * Because stream << and >> will break up strings into words
         * we have this partial specialization here which reads lines.
         */
        template<>
        std::back_insert_iterator<stringlist_t>
        parseSeperatedList<stringlist_t,
                           std::back_insert_iterator<stringlist_t> >
        ( const std::string& s,
          stringlist_t& c,
          std::back_insert_iterator<stringlist_t> out,
          const char sepchar );
        
        template< class Col >
        void parseSeperatedList( const std::string& s,
                                 Col& c,
                                 const char sepchar = ',' )
        {
            parseSeperatedList( s, c, inserter( c, c.end() ), sepchar );
        }
        /**
         * Because stream << and >> will break up strings into words
         * we have this partial specialization here which reads lines.
         */
        template<>
        void
        parseSeperatedList<stringlist_t>( const std::string& s,
                                          stringlist_t& c,
                                          const char sepchar );

        
        /**
         * convert the string 's' which is in the form item1,item2,item3
         * into a string list. The version taking ret as an argument will
         * be faster on large lists because it avoids having to copy
         * the list for the return value
         *
         * @one can create a null seperated list using a non default sepchar
         */
        FERRISEXP_API stringlist_t parseSeperatedList( const std::string& s,
                                         const char sepchar = ',' );
        FERRISEXP_API stringlist_t& parseSeperatedList( const std::string& s,
                                          const char sepchar,
                                          stringlist_t& ret );
        FERRISEXP_API stringset_t& parseSeperatedList( const std::string& s,
                                         const char sepchar,
                                         stringset_t& ret );

        /**
         * convert the list of objects using operator<< in begin, end
         * into a comma seperated string list such that the original data can be obtained
         * using parseSeperatedList() and the same seperator char
         *
         * ie. l == parseSeperatedList( createSeperatedList( l ))
         *
         * @one can create a null seperated list using a non default sepchar
         */
        template <class iterator>
        std::string createSeperatedList( iterator begin,
                                         iterator end,
                                         const char sepchar = ',' )
        {
            std::stringstream ss;
            
            bool virgin = true;

            for( iterator iter = begin; iter != end; ++iter )
            {
                if( virgin ) { virgin = false; }
                else         { ss << sepchar;  }
        
                ss << *iter;
            }

            return ss.str();
        }
        template <class iterator>
        std::string createSeperatedList( iterator begin,
                                         iterator end,
                                         const std::string& sepchar )
        {
            std::stringstream ss;
            
            bool virgin = true;

            for( iterator iter = begin; iter != end; ++iter )
            {
                if( virgin ) { virgin = false; }
                else         { ss << sepchar;  }
        
                ss << *iter;
            }

            return ss.str();
        }
        

        /**
         * convert the list of strings in l into a comma seperated
         * string list such that the original data can be obtained
         * using parseSeperatedList() and the same seperator char
         *
         * ie. l == parseCommaSeperatedList( createCommaSeperatedList( l ))
         *
         * @one can create a null seperated list using a non default sepchar
         */
        FERRISEXP_API std::string createSeperatedList( const stringlist_t& l,
                                                       const char sepchar = ',' );
        FERRISEXP_API std::string createSeperatedList( const stringset_t& l,
                                                       const char sepchar = ',' );
        FERRISEXP_API std::string createSeperatedList( const stringlist_t& l,
                                                       const std::string& sepchar );
        FERRISEXP_API std::string createSeperatedList( const stringset_t& l,
                                                       const std::string& sepchar );

        /**
         * convert the string 's' which is in the form item1,item2,item3
         * into a string list. The version returning the stringlist is
         * deprecated in favor of the version that takes a reference to
         * the collection to insert into. Taking the reference avoids
         * a copy operation.
         *
         * @one can create a null seperated list using a non default sepchar
         */
        FERRISEXP_API stringlist_t parseCommaSeperatedList( const std::string& s );
        FERRISEXP_API void parseCommaSeperatedList( const std::string& s, stringlist_t& sl );
        FERRISEXP_API void parseCommaSeperatedList( const std::string& s, stringset_t& sl );

        /**
         * convert the list of strings in l into a comma seperated
         * string list such that the original data can be obtained
         * using parseCommaSeperatedList()
         *
         * ie. l == parseCommaSeperatedList( createCommaSeperatedList( l ))
         *
         * @one can create a null seperated list using a non default sepchar
         */
        FERRISEXP_API std::string createCommaSeperatedList( const stringlist_t& l );
        FERRISEXP_API std::string createCommaSeperatedList( const stringset_t&  l );

        FERRISEXP_API stringlist_t parseNullSeperatedList( const std::string& s );
        FERRISEXP_API stringlist_t& parseNullSeperatedList( const std::string& s, stringlist_t& ret );
        FERRISEXP_API std::string createNullSeperatedList( stringlist_t& l );

        /**
         * create a new UUID
         */
        FERRISEXP_API std::string makeUUID();
    };

    namespace Time
    {
        const struct tm NullTM = { 0,0,0,0,0,0,0,0,0 };

        struct tm operator+( const struct tm& t1, const struct tm& t2 );

        // convert tm to time_t, mktime()
        FERRISEXP_API time_t toTime( struct tm *tm );
        FERRISEXP_API time_t toTime( struct tm  tm );

        // convert to 1997-07-16T10:30:15+03:00 style
        FERRISEXP_API std::string toXMLDateTime( time_t tt );
        

        FERRISEXP_API struct tm
        ParseTimeString( const std::string& stddatestr,
                         const std::string& defaultformat = "",
                         bool autoFresh = true );

        FERRISEXP_API guint64
        ParseSimpleIntervalString( const std::string& s );

        FERRISEXP_API struct tm&
        FreshenTime( struct tm& tm, struct tm ref = NullTM );
        
        FERRISEXP_API time_t getTime();
        FERRISEXP_API time_t now();
        FERRISEXP_API void Sleep( double t );
        

        FERRISEXP_API void setDefaultTimeFormat( const std::string& s );
        FERRISEXP_API std::string getDefaultTimeFormat();
        /**
         * produce a user readable string from a time value.
         * if desired_format is supplied then that is the format of the string
         * to create, otherwise the default format as provided by
         * getDefaultTimeFormat() will be used
         *
         * The format string provided should be compatible with those accepted
         * by strftime(3)
         */
        FERRISEXP_API std::string toTimeString( time_t, const std::string desired_format = "" );

        /**
         * Convert a number representing hh:mm:ss as a single int count of
         * seconds into the above format string. Note that hh:mm will be
         * omitted if possible.
         */
        FERRISEXP_API std::string toHMSString( int v, bool canOmitMinutes = true );
        

        /**
         * Parse a time string like "yesterday" into a time_t value relative to
         * relv if it is given or getTime() if relv is not supplied.
         *
         * Currently month==30 days and week is taken as 7 days. If you wish to
         * find the start of the previous calendar month use "begin last calmonth"
         * and for the begining of last week "begin last week" or "blast week"
         *
         * Example values for s:
         * "5 minutes ago" or "-5minutes" or "-5min"
         * "1 hour ago" or "-1hr" or "-1h"
         * "4 days ago" or "-4days" or "-4d"
         * "tomorrow" or "+1d"
         * "last month" or "-1mon"
         * "next month"
         * "last quarter" or "-1qu"
         * "3 years ago"
         * "begining last month" "blast month"
         * "blast 6 months"
         * "begin last week"
         * "blast 3 weeks"
         *
         * FIXME: the code for this could use a going over
         *
         * @param s time string to parse
         * @param relv time to base relative offset in 's' on or current time
         *        if no value is given
         * @return a time_t giving the value relv + s as an absolute unix time.
         */
        FERRISEXP_API time_t ParseRelativeTimeString( const std::string& s, time_t relv = 0 );

        /**
         * Class to handle the conversion of strings to integer or time_t
         * values.
         */
        class FERRISEXP_DLLLOCAL RelativeTimeOrIntegerStringParser
        {
            RegexCollection m_convertToIntegerRegexs;
            bool m_wasLastConvertTimeValue;
            
        public:

            RelativeTimeOrIntegerStringParser(
                const std::string& defaultIntegerRegex = "^-?[0-9]+$" );
            
            void addIntegerRegex( const std::string& v );
            guint64 convert( const std::string& v );
            bool wasLastConvertTimeValue();
        };
        

        /**
         * Quick and dirty start/stop timer with a name and a print method.
         * should do down to usec time diffs
         */
        class FERRISEXP_API Benchmark
            :
            public Handlable
        {
            std::string m_name;
            bool m_running;
            fh_ostream m_oss;
            struct timeval m_startTime;
            struct timeval m_stopTime;
    
            void handleError( int eno );
            int  timeval_subtract (struct timeval * result,
                                   struct timeval * x,
                                   struct timeval * y);

        public:
            Benchmark( const std::string& name,
                       bool startAutomatically = true );
            ~Benchmark();
            void start();
            void stop();
            void print();

            float getElapsedTime();

            void setOutputStream( fh_ostream oss );
#define FERRIS_LG_BENCHMARK_D( bm, lsb )                   \
            {                                             \
                Ferris::Timber::_SBufT::Priority_t neededState = Ferris::Timber::_SBufT::PRI_DEBUG;    \
                if( lsb.state() & neededState )                 \
                    bm.setOutputStream( lsb.getRealStream( neededState ) ); \
                else                                            \
                    bm.setOutputStream( Ferris::Factory::fcnull() );    \
            }                                                       \

        };
        FERRIS_SMARTPTR( Benchmark, fh_benchmark );
        
    };

    
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /*** Language Binding Functions *****************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/
    /************************************************************/

    namespace Bindings 
    {
        std::string pread( fh_istream&  iss,   size_t count, size_t offset );
        std::string preadio( fh_iostream&  iss,  size_t count, size_t offset );
        std::string readline( fh_iostream&  iss );
        size_t      pwrite( fh_iostream& oss,  const std::string& str, size_t count, size_t offset );
        size_t      ferriswrite( fh_iostream& oss,  const std::string& str );
        void        fsync( fh_iostream& oss );
        size_t      tellgi ( fh_istream& iss );
        size_t      tellgio( fh_iostream& oss );
        size_t      tellp  ( fh_iostream& oss );
        void        seekgio( fh_iostream& oss, long offset, int whence );
        void        seekp  ( fh_iostream& oss, long offset, int whence );
        bool        goodi ( fh_istream& iss );
        bool        goodio( fh_iostream& oss );
        bool        eofi  ( fh_istream& iss );
        bool        eofio ( fh_iostream& oss );

        ferris_ios::openmode get_ios_in();
        ferris_ios::openmode get_ios_out();
        ferris_ios::openmode get_ios_trunc();
        ferris_ios::openmode get_ios_ate();
        ferris_ios::openmode get_ios_app();

        int get_ios_beg();
        int get_ios_cur();
        int get_ios_end();

        void fireCloseSig( fh_iostream& oss );
        
    };
    
    
    struct PWDScope
    {
        std::string m_olddir;
        PWDScope( const std::string& d );
        ~PWDScope();
    };
    
};
#pragma GCC visibility pop
#endif
