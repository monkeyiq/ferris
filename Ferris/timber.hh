/******************************************************************************
*******************************************************************************
*******************************************************************************

    Copyright (C) 2001 Ben Martin

    This file is part of libferris.

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

    $Id: timber.hh,v 1.2 2010/09/24 21:31:03 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_TIMBER_H_
#define _ALREADY_INCLUDED_TIMBER_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/FerrisHandle.hh>
#include <Ferris/SignalStreams.hh>

#include <syslog.h> 

namespace Ferris
{
    

template<
    typename _CharT,
    typename _Traits = std::char_traits < _CharT >,
    typename _Alloc  = std::allocator   < _CharT >
    >
class ferris_timberbuf
    :
    public ferris_streambuf<_CharT, _Traits>,
    public std::basic_streambuf<_CharT, _Traits>
{
    
    typedef std::basic_streambuf<_CharT, _Traits> sb;

    // Disallow copy/assign
    ferris_timberbuf( const ferris_timberbuf& );
    ferris_timberbuf& operator=( const ferris_timberbuf& );
    
public:
    
    typedef std::char_traits<_CharT>          traits_type;
    typedef typename traits_type::int_type             int_type;
    typedef typename traits_type::char_type            char_type;
    typedef typename traits_type::pos_type             pos_type;
    typedef typename traits_type::off_type             off_type;
    typedef ferris_streambuf<_CharT, _Traits>           _Base;
    typedef ferris_timberbuf<_CharT, _Traits, _Alloc>   _Self;
    typedef std::basic_string<_CharT, _Traits, _Alloc>  _String;


    enum Priority_t {
        PRI_EMERG   = 1<<1,
        PRI_ALERT   = 1<<2,
        PRI_CRIT    = 1<<3,
        PRI_ERR     = 1<<4,
        PRI_WARNING = 1<<5,
        PRI_NOTICE  = 1<<6,
        PRI_INFO    = 1<<7,
        PRI_DEBUG   = 1<<8
    };

    enum Facility_t {
        FAC_KERN    = 1<<1,
        FAC_USER    = 1<<2,
        FAC_MAIL    = 1<<3,
        FAC_DAEMON  = 1<<4,
        FAC_AUTH    = 1<<5,
        FAC_SYSLOG  = 1<<6,
        FAC_LPR     = 1<<7,
        FAC_NEWS    = 1<<8,
        FAC_UUCP    = 1<<9,
        FAC_CRON    = 1<<10,
        FAC_AUTHPRIV= 1<<11,
        FAC_FTP     = 1<<12,

        FAC_L0      = 1<<13,
        FAC_L1      = 1<<14,
        FAC_L2      = 1<<15,
        FAC_L3      = 1<<16,
        FAC_L4      = 1<<17,
        FAC_L5      = 1<<18,
        FAC_L6      = 1<<19,
        FAC_L7      = 1<<20,
    };

    enum Option_t {
        OPT_CONS   = 1<<1,
        OPT_NDELAY = 1<<2,
        OPT_PERROR = 1<<3,
        OPT_PID    = 1<<4
    };

    

    explicit
    ferris_timberbuf( const std::string& ident,
                      int ops = OPT_PID,
                      Facility_t f=FAC_USER,
                      Priority_t p=PRI_WARNING )
        :
        Ident(ident),
        Options(ops),
        offset(0),
        ignoreLogging( false )
        {
//            cerr << "ferris_timberbuf() this:" << (void*)this << endl;
            
            int syslog_fac = 0;
            int syslog_ops = 0;
    
            setFaculty( f );
            setPriority( p );
    
            switch( Option_t(ops) )
            {
            case OPT_CONS:   syslog_ops |= LOG_CONS;   break;
            case OPT_NDELAY: syslog_ops |= LOG_NDELAY; break;
            case OPT_PERROR: syslog_ops |= LOG_PERROR; break;
            case OPT_PID:    syslog_ops |= LOG_PID;    break;
            }

            switch( getFaculty() )
            {
            case FAC_KERN:     syslog_fac |= LOG_KERN;   break;
            case FAC_USER:     syslog_fac |= LOG_USER;   break;
            case FAC_MAIL:     syslog_fac |= LOG_MAIL;   break;
            case FAC_DAEMON:   syslog_fac |= LOG_DAEMON; break;
            case FAC_AUTH:     syslog_fac |= LOG_AUTH;   break;
            case FAC_SYSLOG:   syslog_fac |= LOG_SYSLOG; break;
            case FAC_LPR:      syslog_fac |= LOG_LPR;    break;
            case FAC_NEWS:     syslog_fac |= LOG_NEWS;   break;
            case FAC_UUCP:     syslog_fac |= LOG_UUCP;   break;
            case FAC_CRON:     syslog_fac |= LOG_CRON;   break;
            case FAC_AUTHPRIV: syslog_fac |= LOG_AUTHPRIV; break;
            case FAC_FTP:      syslog_fac |= LOG_FTP;    break;
            case FAC_L0:       syslog_fac |= LOG_LOCAL0; break;
            case FAC_L1:       syslog_fac |= LOG_LOCAL1; break;
            case FAC_L2:       syslog_fac |= LOG_LOCAL2; break;
            case FAC_L3:       syslog_fac |= LOG_LOCAL3; break;
            case FAC_L4:       syslog_fac |= LOG_LOCAL4; break;
            case FAC_L5:       syslog_fac |= LOG_LOCAL5; break;
            case FAC_L6:       syslog_fac |= LOG_LOCAL6; break;
            case FAC_L7:       syslog_fac |= LOG_LOCAL7; break;
            }

            openlog( Ident.c_str(), syslog_ops, syslog_fac);
            
        }
    
    virtual ~ferris_timberbuf()
        {
//            cerr << "ferris_timberbuf() this:" << (void*)this << endl;

            overflow(0);
            closelog();
        }


    const Facility_t getFaculty()  const
        {
            return Faculty;
        }

    const Priority_t getPriority() const
        {
            return Priority;
        }
    
    void setFaculty( Facility_t x )
        {
            Faculty = x;
        }
    
    void setPriority( Priority_t x )
        {
            Priority = x;
        }
    
private:


    Facility_t Faculty;
    Priority_t Priority;
    std::string Ident;
    int    Options;
    fh_stringstream ss; 
    int    offset;
    bool   ignoreLogging;

    int
    getSysLogPriority() const
        {
            int r = 0;
            
            if( getPriority() & PRI_EMERG  ) r |= LOG_EMERG;
            if( getPriority() & PRI_ALERT  ) r |= LOG_ALERT;
            if( getPriority() & PRI_CRIT   ) r |= LOG_CRIT;
            if( getPriority() & PRI_ERR    ) r |= LOG_ERR;
            if( getPriority() & PRI_WARNING) r |= LOG_WARNING;
            if( getPriority() & PRI_NOTICE ) r |= LOG_NOTICE;
            if( getPriority() & PRI_INFO   ) r |= LOG_INFO;
            if( getPriority() & PRI_DEBUG  ) r |= LOG_DEBUG;
            
            return r;
        }
    
protected:

    int overflow(int c)
        {
//            cerr << "timber overflow(c) c:" << (char)c << endl;
            if(c == '\n' || !c)
            {
                if( ! offset ) return c;
                
                offset = 0;

                ss << std::endl;
                ss.seekg(0);

                std::string s="";
                std::getline( ss, s );
//                 cerr << "timber overflow(c) flushing s:" << s << std::endl;
//                 cerr << "timber overflow(c) tostr() :" << tostr(ss) << std::endl;

                if( !ignoreLogging && s.length() )
                {
                    syslog( getSysLogPriority(), " %s", s.c_str());
                }
                
                ss.clear();
                ss.seekp(0);
            }
            else
            {
//                cerr << "timber overflow(c) putting c:" << (char)c << std::endl;
                ss << (char)c;
                ++offset;
            }
            return c;
        }

public:
    
    void setIgnoreLogging( bool v )
        {
            ignoreLogging = v;
        }
};

template<
    typename _CharT,
    typename _Traits = std::char_traits<_CharT>,
    typename _Alloc  = std::allocator   < _CharT >
    >
class ferris_timber
    :
    public Ferris_iostream<_CharT, _Traits>
{
    
    typedef ferris_timberbuf<_CharT, _Traits, _Alloc> ss_impl_t;
    FERRIS_SMARTPTR( ss_impl_t, ss_t );
    ss_t ss;
    typedef Ferris_commonstream<_CharT, _Traits> _CS;

public:

    typedef char                             char_type;
    typedef std::char_traits<char>           traits_type;
    typedef typename traits_type::int_type   int_type;
    typedef typename traits_type::pos_type   pos_type;
    typedef typename traits_type::off_type   off_type;

    typedef emptystream_methods< char_type, traits_type > delegating_methods;
    typedef ferris_timber< _CharT, _Traits, _Alloc> _Self;
    typedef ss_impl_t _SBufT;

    typedef typename ss_impl_t::Facility_t Facility_t;
    typedef typename ss_impl_t::Priority_t Priority_t;
    typedef typename ss_impl_t::Option_t   Option_t;

    explicit ferris_timber( const char* ident = "libferris.so",
                            int ops      = _SBufT::OPT_PID,
                            Facility_t f = _SBufT::FAC_USER,
                            Priority_t p = _SBufT::PRI_WARNING)
        :
        ss( new ss_impl_t( ident, ops, f, p ) )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );

//             cerr << "ferris_timber( ctor ) "
//                  << " this:" << hex << (void*)this
//                  << " sb:" << hex << (void*)sb
//                  << " sh:" << hex << toVoid(sh)
//                  << " rdbuf:" << hex << (void*)Ferris_commonstream< _CharT, _Traits >::rdbuf()
//                  << std::endl;
            
        }
        
    
    ferris_timber( const ferris_timber& rhs )
        :
        ss( rhs.ss )
        {
            this->setsbT( GetImpl(ss) );
            this->init( rdbuf() );

//             cerr << "ferris_timber( & ) "
//                  << " rhs:" << hex << (void*)&rhs
//                  << " this:" << hex << (void*)this
//                  << " sb:" << hex << (void*)sb
//                  << " sh:" << hex << toVoid(sh)
//                  << " rdbuf:" << hex << (void*)Ferris_commonstream< _CharT, _Traits >::rdbuf()
//                  << std::endl;
        }

    ferris_timber& operator=( const ferris_timber& rhs )
        {
            this->setsb( &rhs );
            this->init( _CS::sb );

//             cerr << "ferris_timber( = ) "
//                  << " rhs:" << hex << (void*)&rhs
//                  << " this:" << hex << (void*)this
//                  << " sb:" << hex << (void*)sb
//                  << " sh:" << hex << toVoid(sh)
//                  << " rdbuf:" << hex << (void*)Ferris_commonstream< _CharT, _Traits >::rdbuf()
//                  << std::endl;
            
            this->exceptions( std::ios_base::goodbit );
            clear( rhs.rdstate() );
            copyfmt( rhs );
            return *this;
        }

    
    virtual ~ferris_timber()
        {
//             cerr << "ferris_timber( ctor ) "
//                  << " this:" << hex << (void*)this
//                  << " sb:" << hex << (void*)sb
//                  << " sh:" << hex << toVoid(sh)
//                  << " rdbuf:" << hex << (void*)Ferris_commonstream< _CharT, _Traits >::rdbuf()
//                  << std::endl;
    
        }
    

    _Self* operator->()
         {
             return this;
         }

    ss_impl_t*
    rdbuf() const
        {
            return GetImpl(ss);
        }

    enum
    {
        stream_readable = true,
        stream_writable = false
    };


    const Facility_t getFaculty()  const
        {
            return rdbuf()->getFaculty();
        }

    const Priority_t getPriority() const
        {
            return rdbuf()->getPriority();
        }
    
    void setFaculty( Facility_t x )
        {
            rdbuf()->setFaculty( x );
        }
    
    void setPriority( Priority_t x )
        {
            rdbuf()->setPriority( x );
        }

    /**
     * enable or disable null logging.
     */
    void setIgnoreLogging( bool v )
        {
            rdbuf()->setIgnoreLogging( v );
        }
    
    
};


typedef ferris_timber<char>   Timber;
typedef ferris_timber<char>   f_timber;
typedef ferris_timber<char>   fh_timber;

namespace Factory 
{
    FERRISEXP_API fh_timber& getNullTimber();
};


};


#endif // #ifndef _ALREADY_INCLUDED_TIMBER_H_

