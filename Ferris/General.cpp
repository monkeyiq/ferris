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

    $Id: General.cpp,v 1.25 2010/11/17 21:30:44 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <General.hh>
#include <Trimming.hh>
#include <Enamel.hh>
#include <Resolver.hh>
#include <Ferris.hh>
#include <Ferris_private.hh>
#include <FerrisBoost.hh>

#include <set>

#include <stdlib.h>
#include <unistd.h>
#include <errno.h>

#include <uuid/uuid.h>

#include <config.h>
#include <sys/time.h>

#ifdef HAVE_BOOST
#define BOOST_UTF8_BEGIN_NAMESPACE namespace Ferris {
#define BOOST_UTF8_END_NAMESPACE };
#define BOOST_UTF8_DECL
#include <ThirdParty/boost/utf8_codecvt_facet.hpp>
#include <ThirdParty/boost/utf8_codecvt_facet.cpp>
#endif

#include <iconv.h>

/**********/
/**********/
/**********/

using namespace std;

namespace Ferris
{
    std::string remove_prefix( const std::string& ret,  const std::string& prefix )
    {
        PrefixTrimmer pt;
        pt.push_back( prefix );
        return pt( ret );
    }
    
    std::string remove_postfix( const std::string& ret, const std::string& postfix )
    {
        PostfixTrimmer pt;
        pt.push_back( postfix );
        return pt( ret );
    }
        
    
    std::string tostr( const struct tm& tm )
    {
        fh_stringstream ss;
        ss << "< year:" << tm.tm_year << " month:" << tm.tm_mon
           << " day:" << tm.tm_mday
           << " hour:" << tm.tm_hour
           << " min:" << tm.tm_min
           << " sec:" << tm.tm_sec
           << " />";
        return tostr(ss);
    }


    string toupperstring( const std::string& s )
    {
        std::string ret = s;
        
        for( std::string::iterator p = ret.begin(); p != ret.end(); ++p )
        {
            *p = ::toupper( *p );
        }
        
        return ret;
    }

    string tolowerstring( const std::string& s )
    {
        std::string ret = s;
        
        for( std::string::iterator p = ret.begin(); p != ret.end(); ++p )
        {
            *p = ::tolower( *p );
        }
        
        return ret;
    }
    
    std::string attemptToAbsoluteURL(
        const std::string& maybeRelative,
        const std::string& cwd_const )
    {
        string cwd = cwd_const;
        LG_CTX_D << "attemptToAbsoluteURL() mpath:" << maybeRelative << endl;
        
        int colonidx = maybeRelative.find( ":" );
        if( colonidx == string::npos )
        {
            if( starts_with( maybeRelative, "/" ) )
                return maybeRelative;

            stringstream ss;
            if( cwd.empty() )
                cwd = Shell::getCWDString();
            ss << cwd;
            if( !ends_with( cwd, "/" ) )
                ss << "/";
            ss << maybeRelative;
            LG_CTX_D << "attemptToAbsoluteURL() ret:" << tostr(ss) << endl;
            return tostr(ss);
        }

        LG_CTX_D << "attemptToAbsoluteURL(ret) mpath:" << maybeRelative << endl;
        return maybeRelative;
    }
    
    
    std::basic_ostream< wchar_t, std::char_traits<wchar_t> >&
    operator<<(std::basic_ostream< wchar_t, std::char_traits<wchar_t> >& oss, const std::string& s )
    {
        std::wstring t;
        t = Util::utf8_to_wstring( t, s );
        oss << t;
        return oss;
    }

    std::istream& getline( std::istream& ss, std::string& ret, const std::string& delim )
    {
        stringstream retss;
        char ch;
        while( ss >> noskipws >> ch )
        {
            if( delim.find(ch) != string::npos )
            {
                ss.putback(ch);
                break;
            }
            retss << ch;
        }
        ret = retss.str();
        return ss;
    }

    // std::string chomp( const std::string& s )
    // {
    //     if( s.empty() )
    //         return s;
    //     int p = s.find("\n");
    //     return s.substr( 0, p );
    // }
    std::string chomp( const std::string& s )
    {
        PostfixTrimmer trimmer;
        trimmer.push_back( "\n" );
        trimmer.push_back( "\r" );
        return trimmer( s );
    }
    
    
    stringpair_t
    split( const std::string& data, const std::string& sep )
    {
       int p = data.find( sep );
       string first  = data.substr( 0, p );
       string second = data.substr( p+1 );
       return make_pair( first, second );
    }
   
   
    
    namespace Util
    {
        SingleShot::SingleShot()
            :
            virgin( true ) 
        {}
        
        bool SingleShot::operator()() 
        {
            bool r = virgin;
            virgin = false;
            return r;
        }

        bool SingleShot::value()
        {
            return virgin;
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        typedef wchar_t ucs4_t;

        std::locale& getUTF8Locale()
        {
            static std::locale old_locale;
#ifndef HAVE_BOOST
            return old_locale;
#else
            static std::locale utf8_locale(old_locale,new utf8_codecvt_facet);
//            static std::locale utf8_locale(old_locale,new utf8_codecvt_facet<ucs4_t,char>);
            return utf8_locale;
#endif
        }

        std::string  wstring_to_utf8( const std::wstring& s )
        {
            std::string ret = "";
            wstring_to_utf8( ret, s );
            return ret;
        }
        
        std::string& wstring_to_utf8( std::string& ret, const std::wstring& s )
        {
#ifndef HAVE_BOOST
            cerr << "ERROR: convert_to_wstring() requires boost to work" << endl;
            ret = "";
            return ret;
#else
            typedef mbstate_t ConversionState; // We use the default type

            enum {
                NoError, 
                NoMoreInput,
                ReadError,
                ResultBufferFull,
                ConversionError
            };

            wchar_t* transBuf = (wchar_t*)s.c_str();
            int transBufSize = s.length();

            const int resultBufSize = 1024;
            char resultBuf[resultBufSize];
            char *to = resultBuf;
            char * const toLimit = resultBuf + resultBufSize;
    
            mbstate_t cs;
            wchar_t * fromNext;
            char *toNext;
            std::locale& utf8_locale = getUTF8Locale();

            codecvt_base::result
                convResult = use_facet< codecvt<wchar_t, char, ConversionState> >(utf8_locale).out 
                ((mbstate_t&)cs, (const wchar_t*)transBuf, (const wchar_t*)(transBuf + transBufSize),
                 (const wchar_t*&)fromNext, (char*)to, (char*)toLimit, (char*&)toNext);


            
            if(convResult == codecvt_base::ok)
            {
                string tmp( to, toNext - to );
                ret = tmp;
                return ret;
            }

            ret = "";
            return ret;
#endif
        }
        

        std::wstring utf8_to_wstring( const std::string& s )
        {
            std::wstring ret;
            utf8_to_wstring( ret, s );
            return ret;
        }
        
        
        std::wstring& utf8_to_wstring( std::wstring& ret, const std::string& s )
        {
//             // glib2 attempt
//             const gchar *to_codeset = "utf16";
// //            const gchar *from_codeset = "ISO-8859-1";
//             const gchar *from_codeset = "utf8";
//             gsize bytes_read = 0;
//             gsize bytes_written = 0;
//             GError* error = 0;
            
//             gchar* rstr = g_convert( s.c_str(), s.length()-1,
//                                      to_codeset,
//                                      from_codeset,
//                                      &bytes_read,
//                                      &bytes_written,
//                                      &error );
//             if( rstr )
//             {
//                 cerr << "utf8_convert() bytes_written:" << bytes_written << endl;
// //                wstring tmp( rstr, rstr+bytes_written );
// //                ret = tmp;
//                 ret.resize( bytes_written/2 );
//                 memcpy( (void*)ret.data(), rstr, bytes_written );
//                 g_free( rstr );
//                 return ret;
//             }
//             if( error )
//             {
//                 cerr << "Error at input offset:" << bytes_read << " msg:" << error->message << endl;
//             }
            
                


            
#ifndef HAVE_BOOST
            cerr << "ERROR: utf8_to_wstring() requires boost to work" << endl;
            ret = L"";
            return ret;
#else
            typedef mbstate_t ConversionState; // We use the default type

            enum {
                NoError, 
                NoMoreInput,
                ReadError,
                ResultBufferFull,
                ConversionError
            };

            char* transBuf = (char*)s.c_str();
            int transBufSize = s.length();
            char* transBufEnd = (char*)s.c_str()+transBufSize;

            const int resultBufSize = transBufSize*2+10;
//            const int resultBufSize = 1024;
            wchar_t resultBuf[resultBufSize];
            wchar_t *to = resultBuf;
            wchar_t * const toLimit = (resultBuf + resultBufSize);
    
            mbstate_t cs;
            char * fromNext = 0;
            wchar_t *toNext = 0;
            std::locale& utf8_locale = getUTF8Locale();

//            wofstream wof("/tmp/wof");
            wstringstream wss;
            while( true )
            {
//                 cerr << "Calling in() transBuf:" << (void*)transBuf
//                      << " input-left:" << (transBufEnd-transBuf)
//                      << " to:" << (void*)to
//                      << " to-left:" << (toLimit-to)
//                      << endl;
                codecvt_base::result
                    convResult = use_facet< codecvt<wchar_t, char, ConversionState> >(utf8_locale).in 
                    ((mbstate_t&)cs, (const char*)transBuf, (const char*)(transBufEnd),
                     (const char*&)fromNext, (wchar_t*)to, (wchar_t*)toLimit, (wchar_t*&)toNext);
                
                 if(convResult == codecvt_base::partial)
                 {
                     cerr << "partial conversion. sz:" << (fromNext-transBuf)
                          << " fromNext - start:" << (fromNext - transBuf)
                          << " toNext - to:" << (toNext - to)
                          << endl;
                     wstring tmp( to, toNext - to );
                     wss << tmp;
//                     wof << "tmp:" << tmp << endl;

                     if( fromNext != transBuf )
                     {
                         transBuf = fromNext;
                         to = toNext;
                         continue;
                     }
                 }
                if(convResult == codecvt_base::ok)
                {
                    wstring tmp( to, toNext - to );
                    wss << tmp;
                    ret = wss.str();
                    return ret;
                }
                cerr << "ERROR: utf8_to_wstring() error converting:" << convResult
                     << " partial:" << codecvt_base::partial
                     << " completed:" << (toNext - to)
                     << " transBufSize:" << transBufSize
                     << endl;
                break;
            }
            
            ret = L"";
            return ret;
#endif
        }
        
        std::string iso8859_to_utf8( const std::string& s )
        {
            static iconv_t cd = (iconv_t)-1;
            static bool v = true;
            if( v )
            {
                v = false;
                const char* tocode = "UTF-8";
                const char* fromcode = "ISO-8859-1";
                static iconv_t scd = iconv_open( tocode, fromcode );
                cd = scd;
            }

            if( cd != (iconv_t)-1 )
            {
//                 cerr << "iconv with cd" << endl;
                char*  inbuf = (char*)s.c_str();
                size_t inbytesleft = s.size();
                const size_t outbytessz = 2*inbytesleft + 10;
                size_t outbytesleft = outbytessz;
                char outbuf[ outbytessz+1 ];
                char* p = outbuf;
                
                size_t res = iconv( cd, &inbuf, &inbytesleft, &p, &outbytesleft );
                if( res >= 0 )
                {
                    string ret( outbuf, outbuf+(outbytessz-outbytesleft) );
//                     cerr << "INPUT:" << s << endl;
//                     cerr << "iconv with cd res:" << res << endl;
//                     cerr << "iconv with cd ret:" << ret << endl;
//                     cerr << "outbytessz:" << outbytessz << endl;
//                     cerr << "outbytesleft:" << outbytesleft << endl;
                    return ret;
                }
            }

            //
            // This is a hand made hack for the case where iconv is not desired.
            //
            unsigned char ch = 0;
            stringstream iss;
            stringstream oss;
            iss << s;
            while( iss >> noskipws >> ch )
            {
//                cerr << "iso8859_to_utf8() ch:" << ch << " ch-int:" << (int)ch <<  endl;
                if( ch < 128 )
                    oss << ch;
                else
                {
                    int chi = ch;
                    oss << (char)(((chi&0x7c0)>>6) | 0xc0);
                    oss << (char)((chi&0x3f) | 0x80);
                }
            }
            return tostr(oss);
        }
        
        std::string convert_basename_to_utf8( const std::string& v )
        {
            string ret = g_filename_display_name( v.c_str() );
            return ret;
        }
        

        string convert_url_to_utf8( const std::string& earl )
        {
            string ret = earl;
            
            if( starts_with( earl, "file:" ))
            {
                ret = g_filename_display_name( earl.c_str() );
            }

            return ret;
        }


        std::string URLDecode( const std::string& s )
        {
#define SPC_BASE16_TO_10(x) (((x) >= '0' && (x) <= '9') ? ((x) - '0') : \
                             (toupper((x)) - 'A' + 10))
            stringstream iss;
            iss << s;
            stringstream oss;
            char ch;
            while( iss >> noskipws >> ch )
            {
                if( ch != '%' )
                    oss << ch;
                else
                {
                    char ch2 = 0;

                    if( iss >> noskipws >> ch )
                    {
                        if( iss >> noskipws >> ch2 )
                        {
                            if( isxdigit( ch ) )
                            {
                                if( isxdigit( ch2 ) )
                                {
                                    oss << (char)((SPC_BASE16_TO_10(ch) * 16) + (SPC_BASE16_TO_10(ch2)));
                                    continue;
                                }
                            }
                            oss << ch << ch2;
                        }
                        else
                        {
                            oss << ch;
                        }
                    }
//                 stringstream ess;
//                 ess << "Badly encoded URL s:" << s << endl;
//                 Throw_URLDecodeSyntaxError(ess.str(),0);
                }
            }

            return oss.str();
        
//         string ret = s;
//         ret = Util::replace_all( ret, "%20", " " );
//         return ret;
        }

        std::string EscapeStringAsRegex( const std::string&s )
        {
            string ret = s;
            ret = replace_all( ret, '[', "\\\\[" );
            ret = replace_all( ret, '.', "\\\\." );
            return ret;
        }
        
        
        
        std::string replace_all( const std::string& s, char oldc, char newc )
        {
            string ret;
            for( string::const_iterator iter = s.begin(); iter != s.end(); ++iter )
            {
                if( *iter == oldc ) ret += newc;
                else                ret += *iter;
            }
            return ret;
        }

        std::string replace_all( const std::string& s, char oldc, const std::string& news )
        {
            string ret;
            for( string::const_iterator iter = s.begin(); iter != s.end(); ++iter )
            {
                if( *iter == oldc ) ret += news;
                else                ret += *iter;
            }
            return ret;
        }
        
        std::string replace_all( const std::string& s,
                                 const std::string& olds,
                                 const std::string& news )
        {
            string ret = s;
            int olds_length = olds.length();
            int news_length = news.length();
            
            int start = ret.find( olds );
            while( start != string::npos )
            {
                ret.replace( start, olds_length, news );
                start = ret.find( olds, start + news_length );
            }
            return ret;
        }


        std::string replace_first( const std::string& s,
                                   const std::string& olds,
                                   const std::string& news )
        {
            string ret = s;
            int olds_length = olds.length();
            int news_length = news.length();
            
            int start = ret.find( olds );
            if( start != string::npos )
            {
                ret.replace( start, olds_length, news );
                start = ret.find( olds, start + news_length );
            }
            return ret;
        }
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        static guint64 tryConvertByteString( const std::string& s, int& lastCharUsedPos )
        {
            guint64 rawsize  = 0;
            guint64 shiftfactor = 0;
            string postfix   = "";
            bool parsedPostfix = false;

            fh_stringstream ss;
            ss << s;
            ss >> rawsize;
            lastCharUsedPos = ss.tellg();
            ss >> postfix;

            if( !cmp_nocase( postfix, "k" ) || !cmp_nocase( postfix, "kb" ) )
            {
                parsedPostfix = true;
                shiftfactor = 10;
            }
            else if( !cmp_nocase( postfix, "m" ) || !cmp_nocase( postfix, "mb" ) )
            {
                parsedPostfix = true;
                shiftfactor = 20;
            }
            else if( !cmp_nocase( postfix, "g" ) || !cmp_nocase( postfix, "gb" ) )
            {
                parsedPostfix = true;
                shiftfactor = 30;
            }

//             cerr << "convertByteString( " << s << " ) raw:" << rawsize
//                  << " postfix:" << postfix
//                  << " shift:" << shiftfactor
//                  << " ret:" << ( rawsize << shiftfactor )
//                  << endl;

            if( parsedPostfix )
                lastCharUsedPos = ss.tellg();
            
            return( rawsize << shiftfactor );
        }
        
        
        guint64 convertByteString( const std::string& s )
        {
            int dummy = 0;
            return tryConvertByteString( s, dummy );
        }


        /**
         * Create a human readable string showing the given size
         */
        std::string convertByteString( guint64 s )
        {
            fh_stringstream ss;
            guint64 base=1;
            guint64 blksize = 1024;

            if( s < base*blksize )
            {
                return tostr(s);
            }

            base *= blksize;
            if( s < base*blksize )
            {
                float fs = ((float)s)/base;
                ss.setf( ios_base::fixed, ios_base::floatfield );
                ss.precision(1);
                ss << fs << "k";
                return tostr(ss);
            }

            base *= blksize;
            if( s < base*blksize )
            {
                float fs = ((float)s)/base;
                ss.setf( ios_base::fixed, ios_base::floatfield );
                ss.precision(1);
                ss << fs << "M";
                return tostr(ss);
            }

            base *= blksize;
            if( s < base*blksize )
            {
                float fs = ((float)s)/base;
                ss.setf( ios_base::fixed, ios_base::floatfield );
                ss.precision(2);
                ss << fs << "G";
                return tostr(ss);
            }

            base *= blksize;
            if( s < base*blksize )
            {
                float fs = ((float)s)/base;
                ss.setf( ios_base::fixed, ios_base::floatfield );
                ss.precision(3);
                ss << fs << "T";
                return tostr(ss);
            }

            base *= blksize;
            if( s < base*blksize )
            {
                float fs = ((float)s)/base;
                ss.setf( ios_base::fixed, ios_base::floatfield );
                ss.precision(4);
                ss << fs << "P";
                return tostr(ss);
            }
    
            ss << s;
            return tostr(ss);
        }




        static void
        ParseKeyValueStringPair( const std::string& kv,
                                 StringMap_t& m )
        {
            string k,v;
            fh_stringstream ss;
            ss << kv;
            getline( ss, k, '=' );
            getline( ss, v );
            if( !k.empty() )
                m.insert( make_pair( k, v ));
//            cerr << "ParseKeyValueStringPair k:" << k << " v:" << v << endl;
        }

        StringMap_t&
        ParseKeyValueString( StringMap_t& ret,
                             const std::string& kvs,
                             const std::string& seps )
        {
            string::const_iterator b = kvs.begin();
            string::const_iterator e = kvs.end();
            set< char > seps_set;
            copy( seps.begin(), seps.end(), inserter( seps_set, seps_set.begin() ));
            
            for( ;; )
            {
                string kv;

//                cerr << "STARTING LOOK FOR b:" << b << endl;
                
                // make sure that there is an embedded '=' for the key/value
                // split. This also allows any of the seperators to appear
                // in the key and parsing to work as expected.
                while( b!=e )
                {
                    /* Copy kv pair from string */
                    int oldkvlen = kv.length();
                    
                    copy( b,
                          find_first_of( b, e,
                                         seps.begin(), seps.end(),
                                         equal_to<char>() ),
                          back_insert_iterator< string >(kv) );
                    b += ( kv.length() - oldkvlen );

                    if( b==e || kv.find('=') != string::npos )
                        break;

                    kv.push_back( *b );
                    ++b;
//                     cerr << "Possible embedded sep. kvs:" << kvs << nl
//                          << " kv:" << kv << nl
//                          << " b:" << b << nl
//                          << " e:" << e
//                          << endl;
                }
//                 cerr << "FOUND kv:" << kv << nl
//                      << "            " << " b:" << b << endl;
                
                
                /* Advance past seperators */
                while( seps_set.find( *b ) != seps_set.end() && b != e )
                    ++b;
//                 b = find_first_of( b, e,
//                                    seps.begin(), seps.end(),
//                                    not_equal_to<char>() );

                ParseKeyValueStringPair( kv, ret );

                /* Are we there yet? */
                if( b == e )
                    break;
            }
            return ret;
        }
        
        StringMap_t
        ParseKeyValueString( const std::string& kvs,
                             const std::string& seps )
        {
            StringMap_t ret;
            ParseKeyValueString( ret, kvs, seps );
            return ret;
        }

        StringMap_t
        ParseKeyValueString( fh_istream& iss,
                             const std::string& seps )
        {
            fh_stringstream tss;
            char ch;
            while( iss >> noskipws >> ch )
                tss << ch;
            return ParseKeyValueString( tostr(tss), seps );
        }
        
        
        string
        CreateKeyValueString( const StringMap_t& sm,
                              const std::string kvsep,
                              const std::string atomsep )
        {
            fh_stringstream ss;
            bool v = true;
            for( StringMap_t::const_iterator si=sm.begin(); si!=sm.end(); ++si )
            {
                if( v ) v = false;
                else    ss << atomsep;
                ss << si->first << kvsep << si->second;
            }
            return tostr(ss);
        }
        

        template<>
        std::back_insert_iterator<stringlist_t>
        parseSeperatedList<stringlist_t,
                           std::back_insert_iterator<stringlist_t> >
        ( const std::string& s,
          stringlist_t& c,
          std::back_insert_iterator<stringlist_t> out,
          const char sepchar )
        {
//            cerr << "parseSeperatedList(specialized) s:" << s << endl;
            std::string tmp;

            std::stringstream ss(s);
            while( std::getline( ss, tmp, sepchar ))
                if( !tmp.empty() )
                {
                    *++out = tmp;
                }

            return out;
        }
        template<>
        void
        parseSeperatedList<stringlist_t>( const std::string& s,
                                          stringlist_t& c,
                                          const char sepchar )
        {
            parseSeperatedList( s, c, back_inserter(c), sepchar );
        }
        

        
        

        stringlist_t& parseSeperatedList( const std::string& s,
                                          const char sepchar,
                                          stringlist_t& ret )
        {
            parseSeperatedList( s, ret, sepchar );
            return ret;
            
//             string tmp;
            
//             fh_stringstream ss(s);
//             while( getline( ss, tmp, sepchar ))
//                 if( !tmp.empty() )
//                     ret.push_back( tmp );

//             return ret;
        }
        stringset_t& parseSeperatedList( const std::string& s,
                                         const char sepchar,
                                         stringset_t& ret )
        {
            parseSeperatedList( s, ret, sepchar );
            return ret;
        }
        
        
        stringlist_t parseSeperatedList( const std::string& s, const char sepchar )
        {
            stringlist_t ret;
            return parseSeperatedList( s, sepchar, ret );
        }

        string createSeperatedList( const stringlist_t& l, const char sepchar )
        {
            return createSeperatedList( l.begin(), l.end(), sepchar );
            
//             fh_stringstream ss;
//             bool virgin = true;

//             l.sort();
    
//             for( stringlist_t::iterator iter = l.begin();
//                  iter != l.end(); iter++ )
//             {
//                 if( virgin ) { virgin = false; }
//                 else         { ss << sepchar;  }
        
//                 ss << *iter;
//             }

//             return tostr(ss);
        }
        std::string createSeperatedList( const stringset_t& l, const char sepchar )
        {
            return createSeperatedList( l.begin(), l.end(), sepchar );
        }
        
        std::string createSeperatedList( const stringlist_t& l,
                                         const std::string& sepchar )
        {
            return createSeperatedList( l.begin(), l.end(), sepchar );
        }
        std::string createSeperatedList( const stringset_t& l,
                                         const std::string& sepchar )
        {
            return createSeperatedList( l.begin(), l.end(), sepchar );
        }
        

        stringlist_t parseCommaSeperatedList( const std::string& s )
        {
            return parseSeperatedList( s, ',' );
        }
        void parseCommaSeperatedList( const std::string& s, stringlist_t& sl )
        {
            return parseSeperatedList( s, sl, ',' );
        }
        void parseCommaSeperatedList( const std::string& s, stringset_t& sl )
        {
            return parseSeperatedList( s, sl, ',' );
        }
        string createCommaSeperatedList( const stringlist_t& l )
        {
            return createSeperatedList( l, ',' );
        }
        string createCommaSeperatedList( const stringset_t& l )
        {
            return createSeperatedList( l, ',' );
        }
        stringlist_t parseNullSeperatedList( const std::string& s )
        {
            return parseSeperatedList( s, '\0' );
        }
        stringlist_t& parseNullSeperatedList( const std::string& s, stringlist_t& ret )
        {
            parseSeperatedList( s, '\0', ret );
            return ret;
        }
        std::string createNullSeperatedList( stringlist_t& l )
        {
            return createSeperatedList( l, '\0' );
        }
        
        
        std::string makeUUID()
        {
            char buf[40];
            uuid_t uu;
            uuid_generate_time( uu );
            uuid_unparse( uu, buf );
            return buf;
        }
    };
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////
        ///////////////////////////////////////////////////////////////////////////////

    namespace Time
    {
        template <class charT, class Traits>
        basic_ostream<charT, Traits>&
        operator<< (basic_ostream<charT, Traits>& os, const struct tm& tm )
        {
            os << "< year:" << tm.tm_year << " month:" << tm.tm_mon
               << " day:" << tm.tm_mday
               << " hour:" << tm.tm_hour
               << " min:" << tm.tm_min
               << " sec:" << tm.tm_sec
               << " />";
            return os;
        }

        
        /**
         * Update any unfilled in parts of the given tm with data from the
         * reference time given. Note that the parameter tm is updated in
         * this function.
         *
         * @param tm time to update
         * @param ref reference time to copy data from or the current time.
         * @returns The updated time
         */
        struct tm&
        FreshenTime( struct tm& tm, struct tm ref )
        {
            struct tm tmp;
            
            if( !ref.tm_year )
            {
                time_t tt = getTime();
                tmp = *(localtime( &tt ));
                ref = tmp;
            }
            
            if( -1 == tm.tm_sec )  tm.tm_sec  = ref.tm_sec;
            if( -1 == tm.tm_min )  tm.tm_min  = ref.tm_min;
            if( -1 == tm.tm_hour ) tm.tm_hour = ref.tm_hour;
            if( -1 == tm.tm_mday ) tm.tm_mday = ref.tm_mday;
            if( -1 == tm.tm_mon )  tm.tm_mon  = ref.tm_mon;
            if( -1 == tm.tm_year ) tm.tm_year = ref.tm_year;
            return tm;
        }

        /**
         * A wrapper for time(2) that throws and exception on the rare case that
         * time(2) fails.
         */
        time_t getTime()
        {
            time_t tt = time( 0 );
            if( tt == -1 )
            {
                int eno = errno;
                fh_stringstream ss;
                ss << "can not get time with time(2)";
                ThrowFromErrno( eno, tostr(ss), 0 );
            }
            return tt;
        }

        time_t now()
        {
            return getTime();
        }
        
        

        /**
         * Sleep the given number of seconds and fractions of seconds.
         */
        void Sleep( double t )
        {
            unsigned int usecs = 0;
            t *= 1000000;
            usecs = (unsigned int)t;
            usleep(usecs);
        }
        

        
        class FERRISEXP_DLLLOCAL EnvironmentAssignmentIsInit
        {
            string k;
            string v;
            bool wasSet;
            
        public:
            EnvironmentAssignmentIsInit( const std::string& s )
                :
                k( s ),
                wasSet( false )
                {
                    const char* vc = getenv( k.c_str() );
                    if( vc )
                    {
                        wasSet = true;
                        v = vc;
                    }
                }
            ~EnvironmentAssignmentIsInit()
                {
                    if( wasSet )
                        setenv( k.c_str(), v.c_str(), true );
                }
        };
        

        struct tm
        operator+( const struct tm& t1, const struct tm& t2 )
        {
            struct tm ret;
            bzero( &ret, sizeof(ret));
    
            ret.tm_year = t1.tm_year + t2.tm_year;
            ret.tm_mon  = t1.tm_mon  + t2.tm_mon;
            ret.tm_mday = t1.tm_mday + t2.tm_mday;
            ret.tm_hour = t1.tm_hour + t2.tm_hour;
            ret.tm_min  = t1.tm_min  + t2.tm_min;
            ret.tm_sec  = t1.tm_sec  + t2.tm_sec;
    
            return ret;
        }

        time_t toTime( struct tm *tm )
        {
            return mktime( tm );
        }
        time_t toTime( struct tm tm )
        {
            return mktime( &tm );
        }

        std::string toXMLDateTime( time_t tt )
        {
            return toTimeString( tt, "%Y-%m-%dT%H:%M:%SN%z" );
        }
        
        
        
        /**
         * Convert a time string into a struct tm trying first the default format
         * given and then a list of predefined possible data matches.
         *
         * As of 0.9.71 the autoFresh param was added. The default is to call
         * FreshenTime() on the return value before returning. This allows time
         * relative to now to be parsed, for example, 9:30 and the day, year, month
         * info is gleemed from the current time.
         * Note that the old style of exact match or -1 in data feild can still be
         * obtained using autoFresh=false. In this case the returned tm struct may be
         * missing information if that is not present in stddatestr. One can use
         * FreshenTime to fill in missing data from the return value with data from any
         * given reference time.
         *
         * Note that if autoFresh==false, any data that is not present in the stddatestr
         * given is set to -1. So a call to FreshenTime() is highly advised to set those
         * to a valid state.
         *
         * @param stddatestr Date string to parse
         * @param defaultformat Format string to parse the data string with as a first
         *        attempt. If that fails then a list of other date formats is attempted.
         * @param autoFresh if true (the default) then FreshenTime() is called on the
         *        date before return
         *
         * @seealso FreshenTime()
         * @throws Throw_BadlyFormedTimeString if stddatestr can not be parsed with any
         * of the default time/date formats
         */
        struct tm
        ParseTimeString( const std::string& stddatestr,
                         const std::string& defaultformat,
                         bool autoFresh )
        {
            LG_TIME_D << "ParseTimeString() stddatestr:" << stddatestr << endl;
            
            const char* datestr = stddatestr.c_str();
            const char* eos     = datestr + strlen( datestr );
            struct tm tm;

            typedef vector<string> formats_t;
            formats_t formats;

            string httpdate = "%a, %e %b %Y %H:%M:%S";

            if( !defaultformat.empty() )
                formats.push_back( defaultformat );
            formats.push_back( httpdate );
            formats.push_back( "%a %b %e %H:%M:%S +0000 %Y" );
            formats.push_back( "%y %b %e %H:%M:%S" );
            formats.push_back( "%y %b %e %H:%M" );
            formats.push_back( "%y %e %b %H:%M:%S" );
            formats.push_back( "%y %e %b %H:%M" );
            formats.push_back( "%Y %b %e %H:%M:%S" );
            formats.push_back( "%Y %b %e %H:%M" );
            formats.push_back( "%Y %e %b %H:%M:%S" );
            formats.push_back( "%Y %e %b %H:%M" );
            formats.push_back( "%b %e %H:%M:%S" );
            formats.push_back( "%b %e %H:%M" );
            formats.push_back( "%e %b %H:%M:%S" );
            formats.push_back( "%e %b %H:%M" );
            formats.push_back( "%Y-%m-%dT%H:%M:%S%z" );
            formats.push_back( "%Y-%m-%dT%H:%M:%SZ" );
            formats.push_back( "%Y-%m-%dT%H:%M:%S" );
            formats.push_back( "%Y-%m-%d %H:%M:%S" );
            formats.push_back( "%Y:%m:%d %H:%M:%S" );
            formats.push_back( "%d/%m/%y %H:%M:%S" );
            formats.push_back( "%d/%m/%y %H:%M" );
            formats.push_back( "%d/%m/%Y %H:%M:%S" );
            formats.push_back( "%d/%m/%Y %H:%M" );
            formats.push_back( "%d/%m %H:%M:%S" );
            formats.push_back( "%d/%m %H:%M" );
            formats.push_back( "%b%n%d%n%H:%M" );
            formats.push_back( "%a%n%H:%M" );
            formats.push_back( "%d%n%H:%M" );
            formats.push_back( "%Y-%m-%d" );
            formats.push_back( "%H:%M:%S" );
            formats.push_back( "%H:%M" );

            for( formats_t::iterator iter = formats.begin(); iter != formats.end(); ++iter )
            {
                string format = *iter;

//                 cerr << "ParseTimeString() stddatestr:" << stddatestr << endl;
//                 cerr << "ParseTimeString() format:" << format << endl;
                
                memset( &tm, 0, sizeof(struct tm));
                tm.tm_sec  = -1;
                tm.tm_min  = -1;
                tm.tm_hour = -1;
                tm.tm_mday = -1;
                tm.tm_mon  = -1;
                tm.tm_year = -1;
                const char* rc = strptime( datestr, format.c_str(), &tm );
                LG_TIME_D << "ParseTimeString() stddatestr:" << stddatestr
                          << " format:" << format
                          << " rc:" << (void*)rc
                          << " eos:" << (void*)eos
                          << endl;

                if( rc == eos )
                {
                    LG_TIME_D << "ParseTimeString() stddatestr:" << stddatestr
                              << " got struct tm:" << tm
                              << endl;
                    FreshenTime( tm );
                    return tm;
                }

                /*
                 * PURE DEBUG
                 */
                if( *iter == httpdate )
                {
                    int e = errno;
                    string es = errnum_to_string( "", e );
                    LG_TIME_D << " iter is httpdate rc:" << (rc?rc:"null")
                              << " es:" << es << endl
                              << " date:" << datestr
                              << " format:" << format
                              << endl;

                    char buf[255];
                    string sf = format;
                    sf += " - Z:%Z - z:%z";
                    strftime(buf, sizeof(buf), sf.c_str(), &tm);
//                    cerr << buf << endl;
                }
                
                
                /*
                 * Handle tagging of GMT and TZ data explicitly, %Z doesn't
                 * work in most places.
                 */
                if( *iter == httpdate && rc )
                {
                    string tz;
                    
                    stringstream restss;
                    restss << rc;
                    restss >> tz;

                    if( tz.empty() )
                    {
                        LG_TIME_D << "ParseTimeString() http date without TZ info "
                                  << " stddatestr:" << stddatestr
                                  << endl;
                        continue;
                    }

                    EnvironmentAssignmentIsInit tzrevert("TZ");

                    stringstream tzss;
                    tzss << ":" << tz << endl;
                    setenv( "TZ", tostr( tzss ).c_str(), true );

                    /*
                     * Trim off the %Z timezone data
                     */
                    string truncTime = stddatestr.substr( 0, rc - datestr );
                    const char* begintt = truncTime.c_str();
                    
                    rc = strptime( begintt, format.c_str(), &tm );
                    if( rc == begintt + strlen( begintt ) )
                    {
                        LG_TIME_D << "ParseTimeString() stddatestr:" << stddatestr
                                  << " got struct tm:" << tm
                                  << endl;
                        FreshenTime( tm );
                        return tm;
                    }                    
                }
            }

            fh_stringstream ss;
            ss << "Can not parse given time:" << stddatestr;
            Throw_BadlyFormedTimeString( tostr(ss), 0 );
        }

        static bool UsingFormatOverRide = false;
        static string FormatOverRide = "";

        /**
         * Override the users default preference for date/time presentation
         */
        void setDefaultTimeFormat( const std::string& s )
        {
            UsingFormatOverRide = true;
            FormatOverRide = s;
            LG_CTX_D << "SL_setTimeStrFTimeStream() FormatOverRide:" << FormatOverRide << endl;
            
        }

        /**
         * Get the users preference for date/time presentation
         */
        std::string getDefaultTimeFormat()
        {
            if( UsingFormatOverRide )
            {
                return FormatOverRide;
            }
            return getEDBString( FDB_GENERAL, "strftime-format-string", "%y %b %e %H:%M" );
        }

        /**
         * Format a time_t to a string using the users desired date/time presentation
         * options.
         */
        std::string toTimeString( time_t TT, const std::string desired_format )
        {
            const int bufmaxlen = 1025;
            char buf[bufmaxlen];
            struct tm* TM = 0;
            string format = desired_format.empty() ? getDefaultTimeFormat() : desired_format;

            TM = localtime( &TT );
            
            if( TM && strftime( buf, bufmaxlen, format.c_str(), TM) )
            {
                string s = buf;
                return s;
            }
            // FIXME
            return "";
        }

        string toHMSString( int v, bool canOmitMinutes )
        {
            bool haveLargerUnit = false;
            stringstream ret;
            if( v >= 3600 )
            {
                int h = (v-(v%3600))/3600;
                v -= h * 3600;
                ret << setfill('0') << setw(2) << h << ":";
                haveLargerUnit = true;
            }
            if( v >= 60 || !canOmitMinutes )
            {
                int m = (v-(v%60))/60;
                v -= m * 60;
                if( haveLargerUnit )
                    ret << setfill('0') << setw(2);
                ret << m << ":";
                haveLargerUnit = true;
            }
            ret << setfill('0') << setw(2) << (v%60);

            return ret.str();
        }
        

        /**
         * Parse a string like; (note 0.25 is 250 millis)
         *
         * 0.25
         * 10s
         * 30m
         * 2h
         * 5d
         *
         * into a integer representing the number of milliseconds until that interval
         * will have come to pass
         *
         * @param s like the above interval string
         * @return number of millis represented by s
         */
        guint64
        ParseSimpleIntervalString( const std::string& s )
        {
            if( s.find(".") )
            {
                double f = toType<double>(s);
                f *= 1000;
                return guint64(f);
            }
            
            guint64 ret = toint(s) * 1000;
            char postfix = s[ s.length() - 1 ];

            switch( postfix )
            {
            case 's':                   break;
            case 'm': ret *= 60;        break;
            case 'h': ret *= 3600;      break;
            case 'd': ret *= 24 * 3600; break;
            default:                    break;
            }
            return ret;
        }

        /**
         * Parse a string unit time into the number of seconds it represents
         */
        static time_t unitToDelta( const std::string& unit )
        {
            time_t delta = 0;
            const time_t minute = 60;
            const time_t hour   = 60 * minute;
            const time_t day    = 24 * hour;
            const time_t week   = 7 * day;
            const time_t qu     = 13 * week;
            const time_t year   = 52 * week;

            if( contains( unit, "minute" ) || contains( unit, "min" ))
                delta = minute;
            else if( contains( unit, "hour" ) )
                delta = hour;
            else if( contains( unit, "day" ) )
                delta = day;
            else if( contains( unit, "week" ) )
                delta = week;
            else if( contains( unit, "month" ) )
                delta = 30*day;
            else if( contains( unit, "qu" ) )
                delta = qu;
            else if( contains( unit, "year" ) )
                delta = year;
            else if( contains( unit, "h" ) )
                delta = hour;
            else if( contains( unit, "d" ))
                delta = day;
            else if( contains( unit, "w" ))
                delta = week;
            else if( contains( unit, "qu" ) )
                delta = qu;
            else if( contains( unit, "y" ))
                delta = year;
            
            return delta;
        }

        static void moveByYears( struct tm& mytime, int yearOffset )
        {
            mytime.tm_year += yearOffset;
            time_t tt = mktime( &mytime );
            mytime = *(localtime( &tt ));
        }
        
        
        static void moveByMonths( struct tm& mytime, int monthOffset )
        {
            mytime.tm_mon += monthOffset;
            time_t tt = mktime( &mytime );
            mytime = *(localtime( &tt ));
            
//             int yrs = abs( monthOffset ) % 12;
//             if( monthOffset<0 ) yrs *= -1;
            
//             mytime.tm_year += yrs;
//             monthOffset    -= yrs;
//             int delta       = monthOffset > 1 ? 1 : -1;
//             while( monthOffset != 0 )
//             {
//                 if( mytime.tm_mon > 0 && mytime.tm_mon < 11 ) 
//                     mytime.tm_mon--;
//                 else
//                 {
//                     mytime.tm_mon = 11;
//                     mytime.tm_year--;
//                 }
//             }
        }

        static void moveByDays( struct tm& mytime, int dayOffset )
        {
            mytime.tm_mday += dayOffset;
            time_t tt = mktime( &mytime );
            mytime = *(localtime( &tt ));
        }
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/


        RelativeTimeOrIntegerStringParser::RelativeTimeOrIntegerStringParser(
            const std::string& defaultIntegerRegex )
            :
            m_wasLastConvertTimeValue( false )
        {
            addIntegerRegex( defaultIntegerRegex );
        }
            
            
        void
        RelativeTimeOrIntegerStringParser::addIntegerRegex( const std::string& v )
        {
            m_convertToIntegerRegexs.append( v );
        }

//         static long tointX( const std::string& s )
//             {
//                 LG_EAIDX_D << "tointX() s:" << s << endl;
//                 long ret;
//                 ::Ferris::fh_stringstream ss(s);
//                 ss >> ret;
//                 return ret;

// //                 int ret;
// //                 std::stringstream ss(s);
// //                 ss >> ret;
// //                 return ret;
//             }
        
        guint64
        RelativeTimeOrIntegerStringParser::convert( const std::string& v )
        {
            m_wasLastConvertTimeValue = false;

            if( fh_rex r = m_convertToIntegerRegexs.getRegex() )
            {
                if( regex_match( v, r, boost::match_any ) )
                {
//                     LG_EAIDX_D << "toint(3.2) v:" << toint(v) << endl;
//                     LG_EAIDX_D << "toint(3.3) v:" << tointX(v) << endl;
//                     LG_EAIDX_D << "toint(3.ret) v:" << toType<guint64>( v ) << endl;
//                    cerr << "RelativeTimeOrIntegerStringParser::convert(int) v:" << v << endl;
                    return toType<guint64>( v );
                }
            }

            int lastCharUsedPos = 0;
            guint64 sz = Util::tryConvertByteString( v, lastCharUsedPos );
//             cerr << "RelativeTimeOrIntegerStringParser::convert(bytestring) v:" << v << ":" << endl;
//             cerr << "RelativeTimeOrIntegerStringParser::convert(bytestring) sz:" << sz << endl;
//             cerr << "RelativeTimeOrIntegerStringParser::convert(bytestring) lastCharUsedPos:" << lastCharUsedPos << endl;
//             cerr << "RelativeTimeOrIntegerStringParser::convert(bytestring) len:" << v.length() << endl;
//            cerr << "str.length:" << v.length() << " lastchar:" << lastCharUsedPos << endl;
            if( lastCharUsedPos != v.length() ) // !sz && v != "0" )
            {
                time_t tt = Time::ParseRelativeTimeString( v );
                sz = tt;
                m_wasLastConvertTimeValue = true;
            }
            return sz;
        }
        
        bool
        RelativeTimeOrIntegerStringParser::wasLastConvertTimeValue()
        {
            return m_wasLastConvertTimeValue;
        }
        
        
        
        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/

        Benchmark::Benchmark( const std::string& name,
                              bool startAutomatically )
            :
            m_name( name )
        {
            setOutputStream( Factory::fcerr() );
            if( startAutomatically )
                start();
        }

        Benchmark::~Benchmark()
        {
            if( m_running )
            {
                stop();
                print();
            }
        }

        void
        Benchmark::handleError( int eno )
        {
            cerr << "got an error:" << eno << endl;
        }

        int
        Benchmark::timeval_subtract (struct timeval * result,
                                     struct timeval * x,
                                     struct timeval * y)
        {
            /* Perform the carry for the later subtraction by updating Y. */
            if (x->tv_usec < y->tv_usec) {
                int nsec = (y->tv_usec - x->tv_usec) / 1000000 + 1;
                y->tv_usec -= 1000000 * nsec;
                y->tv_sec += nsec;
            }
            if (x->tv_usec - y->tv_usec > 1000000) {
                int nsec = (x->tv_usec - y->tv_usec) / 1000000;
                y->tv_usec += 1000000 * nsec;
                y->tv_sec -= nsec;
            }
                                                                                                                     
            /* Compute the time remaining to wait.
               `tv_usec' is certainly positive. */
            result->tv_sec = x->tv_sec - y->tv_sec;
            result->tv_usec = x->tv_usec - y->tv_usec;
                                                                                                                     
            /* Return 1 if result is negative. */
            return x->tv_sec < y->tv_sec;
        }

        void
        Benchmark::start()
        {
            m_running = true;
            int rc = gettimeofday( &m_startTime, 0 );
            if( rc == -1 )
                handleError( errno );
        
        }

        void
        Benchmark::stop()
        {
            int rc = gettimeofday( &m_stopTime, 0 );
            m_running = false;
            if( rc == -1 )
                handleError( errno );
        }

        void
        Benchmark::setOutputStream( fh_ostream oss )
        {
            m_oss = oss;
        }

        void
        Benchmark::print()
        {
            if( m_running )
                stop();

            struct timeval diff;
            timeval_subtract( &diff, &m_stopTime, &m_startTime );
            int sec = diff.tv_sec;
            int msec = diff.tv_usec/1000;
            int mins = (int)floor( diff.tv_sec / 60.0 );
            m_oss << m_name;
            if( mins > 0 )
            {
                sec = sec - (60*mins);
                m_oss << " completed in " << mins << " mins and "
                      << setw(2) << setfill('0') << sec;
            }
//             else if( sec > 0 )
//             {
//                 m_oss << " completed in sec:" << sec;
//                 if( sec < 5 )
//                     m_oss << " and "
//                           <<  setw(2) << setfill('0') << msec
//                           << " milliseconds." << flush;
//             }
            else
            {
                m_oss << " completed in sec:" << sec;
            }
            
            m_oss << '.' << setw(3) << setfill('0') << msec << endl;
/////            << " usec:" << diff.tv_usec << endl;
        }

        float
        Benchmark::getElapsedTime()
        {
            if( m_running )
                stop();

            struct timeval diff;
            timeval_subtract( &diff, &m_stopTime, &m_startTime );
            int msec = diff.tv_usec/1000;
            float ret = diff.tv_sec + (msec / 1000.0);
            return ret;
        }
        
        

        /********************************************************************************/
        /********************************************************************************/
        /********************************************************************************/
        
    };

    namespace Bindings 
    {

        std::string pread( fh_istream&  iss, size_t count, size_t offset )
        {
            std::string str = "";
            str.resize( count + 1 );
            iss.seekg( offset, ios_base::beg );
            iss.read( (char*)str.c_str(), count );
            size_t c = iss.gcount();
            str[ c ] = '\0';
            str.resize( c );
            return str;
        }

        std::string preadio( fh_iostream&  iss, size_t count, size_t offset )
        {
            std::string str = "";
            str.resize( count + 1 );
            iss.seekg( offset, ios_base::beg );
            iss.read( (char*)str.c_str(), count );
            size_t c = iss.gcount();
            str[ c ] = '\0';
            str.resize( c );
            return str;
        }
        std::string readline( fh_iostream&  iss )
        {
            std::string ret;
            getline( iss, ret );
            return ret;
        }
        
        void seekgio( fh_iostream& oss, long offset, int whence )
        {
            oss.seekg( offset, (ios_base::seekdir)whence );
        }
    
    
        void seekp( fh_iostream& oss, long offset, int whence )
        {
            oss.seekp( offset, (ios_base::seekdir)whence );
        }
        
        
        
        size_t pwrite( fh_iostream& oss, const std::string& str, size_t count, size_t offset )
        {
            oss.seekp( offset, ios_base::beg );
            oss.write( str.c_str(), count );
            if( oss.bad() )
                return 0;
            return count;
        }
        
        size_t ferriswrite( fh_iostream& oss,  const std::string& str )
        {
            size_t count = str.length();
            oss.write( str.c_str(), count );
            if( oss.bad() )
                return 0;
            return count;
        }

        void fsync( fh_iostream& oss )
        {
            oss << flush;
        }

        size_t tellgi( fh_istream& iss )
        {
            size_t ret = iss.tellg();
            return ret;
        }
        
        size_t tellgio( fh_iostream& oss )
        {
            size_t ret = oss.tellg();
            return ret;
        }
        
        size_t tellp ( fh_iostream& oss )
        {
            size_t ret = oss.tellp();
            return ret;
        }
        bool        goodi ( fh_istream& iss )
        {
            return iss.good();
        }
        
        bool        goodio( fh_iostream& oss )
        {
            return oss.good();
        }
        
        bool        eofi  ( fh_istream& iss )
        {
            return iss.eof();
        }
        
        bool        eofio ( fh_iostream& oss )
        {
            return oss.eof();
        }
        
        ferris_ios::openmode get_ios_in()
        {
            return ios::in;
        }
        
        ferris_ios::openmode get_ios_out()
        {
            return ios::out;
        }
        
        ferris_ios::openmode get_ios_trunc()
        {
            return ios::trunc;
        }
        
        ferris_ios::openmode get_ios_ate()
        {
            return ios::ate;
        }
        ferris_ios::openmode get_ios_app()
        {
            return ios::app;
        }
        
        int get_ios_beg()
        {
            return ios_base::beg;
        }
        
        int get_ios_cur()
        {
            return ios_base::cur;
        }
        int get_ios_end()
        {
            return ios_base::end;
        }

        void fireCloseSig( fh_iostream& oss )
        {
            oss.getCloseSig().emit( oss, oss.tellp() );
            
        }
        
        
    };


    
    PWDScope::PWDScope( const std::string& d )
    {
        char buf[ PATH_MAX + 1 ];
        getwd( buf );
        m_olddir = buf;
        chdir( d.c_str() );
    }
    
    PWDScope::~PWDScope()
    {
        chdir( m_olddir.c_str() );
    }
    
    
};
