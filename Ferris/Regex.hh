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

    $Id: Regex.hh,v 1.5 2010/09/24 21:30:57 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_REGEX_H_
#define _ALREADY_INCLUDED_FERRIS_REGEX_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
//#include <Ferris/FerrisBoost.hh>
#include <SmartPtr.h>

namespace boost {
    namespace serialization {
        class access;
    }
}

namespace Ferris
{
    

//     class Regex;
//     FERRIS_SMARTPTR( Regex, fh_regex );
    
};


#include <Ferris/Ferris.hh>
#include <regex.h>

#include <boost/regex.hpp>

namespace Ferris
{
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_API Regex : public Handlable // , public Object
{
    friend class FerrisRegExCompileError;

    fh_rex m_regex;
//    regex_t RE;
    std::string REstr;
    int m_cflags;
    boost::regex::flag_type m_rflags;
    
    friend std::string getErrorString( int e, const Regex& r);

    friend class boost::serialization::access;
    template<class Archive >
    void serialize(Archive & ar, const unsigned int version)
        {
            ar & REstr;
            ar & m_cflags;
            // after load, setup regex state.
            this->setup();
        }
    
    void setup();

public:

//    Regex( const std::string& re, int cflags = -1 );
    Regex( const std::string& re, boost::regex::flag_type rflags = boost::regex::optimize );
    ~Regex();

//    bool operator()( const std::string& s, int EFLAGS ) const;
    bool operator()( const std::string& s ) const;
    

};


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

class FERRISEXP_EXCEPTION FerrisRegExCompileError
    :
        public FerrisVFSExceptionBase
{
    std::string ErrorString;
    
public:

    FerrisRegExCompileError(
        const FerrisException_CodeState& state,
        fh_ostream log,
        const std::string& errorString,
        Attribute* a=0);

    ~FerrisRegExCompileError() throw ()
        {
        }

    
    
};
#define Throw_FerrisRegExCompileError(estr) \
throw FerrisRegExCompileError( \
FerrisException_CodeState( __FILE__ ,  __LINE__ , __PRETTY_FUNCTION__ ), \
(Enamel::get__t_l1w()), (estr), 0)


 
};

#endif // #ifndef _ALREADY_INCLUDED_FERRIS_REGEX_H_





