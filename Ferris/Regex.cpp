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

    $Id: Regex.cpp,v 1.5 2010/09/24 21:30:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/
#include <config.h>

#include <Regex.hh>
#include <Ferris/FerrisBoost.hh>

using namespace std;

namespace Ferris
{
//     static char *get_regerror (int errcode, const regex_t *compiled)
//     {
//         size_t length = regerror (errcode, compiled, NULL, 0);
//         char *buffer = (char*)malloc (length);
//         (void) regerror (errcode, compiled, buffer, length);
//         return buffer;
//     }

//     string
//     getErrorString( int e, const Regex& r)
//     {
//         char* estr = get_regerror( e, &r.RE );
//         string ErrorString = estr;
//         free(estr);
//         return ErrorString;
//     }


    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    void Regex::setup()
    {
        if( REstr.empty() )
            m_regex = 0;
        else
            m_regex = toregexh( REstr, m_rflags );
        
//         memset( &RE, 0, sizeof(regex_t) );
//         if( m_cflags == -1 )
//         {
//             m_cflags = REG_EXTENDED | REG_NOSUB;// | REG_NEWLINE;
//         }
    
//         switch( int rc = regcomp( &RE, REstr.c_str(), m_cflags) )
//         {
//         case 0: // success
//             break;

//         default:
// //        cerr << "Regex.cpp: Invalid regex given:" << re << endl;
//             Throw_FerrisRegExCompileError( getErrorString( rc, *this )  );
//         }
    }
    
//     Regex::Regex( const string& re, int cflags)
//         :
//         REstr(re),
//         m_cflags( cflags )
//     {
// //    cerr << "Regex::Regex() re:" << re << endl;
//         setup();
//     }


    Regex::Regex( const std::string& re, boost::regex::flag_type rflags )
        :
        REstr(re),
        m_cflags( -1 ),
        m_regex( 0 ),
        m_rflags( rflags )
    {
//    cerr << "Regex::Regex() re:" << re << endl;
        setup();
    }
        
    
    Regex::~Regex()
    {
//        regfree( &RE );

    }


//     bool
//     Regex::operator()( const string& s, int EFLAGS ) const
//     {
    
//         int rc = regexec( &RE, (char*)s.c_str(), 0, NULL, EFLAGS );

//         switch(rc)
//         {
//             /*
//              * Matched
//              */
//         case 0:
//             return true;

//         case REG_NOMATCH:
//             return false;

//         case REG_ESPACE:
//             Throw_FerrisOutOfMemory("",0);

//         default:
//             break;
//         }
//         Throw_FerrisRegExCompileError( getErrorString( rc, *this )  );
//     }


    bool
    Regex::operator()( const string& s ) const
    {
        if( !m_regex )
            return false;
        
        return regex_search( s, m_regex, boost::match_any );
    }




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

 
FerrisRegExCompileError::FerrisRegExCompileError(
    const FerrisException_CodeState& state,
    fh_ostream log,
    const std::string& errorString,
    Attribute* a )
    :
    FerrisVFSExceptionBase( state, log, errorString.c_str(), a ),
    ErrorString( errorString )
{
    setExceptionName("FerrisRegExCompileError");
}



 
};
