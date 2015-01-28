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

    $Id: FerrisXalan.cpp,v 1.2 2010/09/24 21:30:45 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/FerrisXalan_private.hh>

#include <XalanDOM/XalanDOMException.hpp>
#include <PlatformSupport/XSLException.hpp>
#include <XalanTransformer/XalanTransformer.hpp>
#include <XercesParserLiaison/XercesDOMSupport.hpp>
#include <XercesParserLiaison/XercesParserLiaison.hpp>
#include <XalanTransformer/XercesDOMWrapperParsedSource.hpp>

using namespace std;
using namespace XALAN_CPP_NAMESPACE;

namespace Ferris
{
    XalanDOMString domstr( const std::string& s )
    {
        return XalanDOMString( s.c_str() );
    }
    
    std::string tostr( const XalanDOMString& ds )
    {
#if XALAN_VERSION_MAJOR == 1 & XALAN_VERSION_MINOR == 8
        XalanDOMString::CharVectorType v = ds.transcode();
        std::string ret( v.begin(), v.end()-1 );
#else
        XalanDOMString::CharVectorType v;
        ds.transcode( v );
        std::string ret( v.begin(), v.end()-1 );
#endif
        
//         if( !ret.empty() && ret[ ret.length() ] == '\0' )
//             ret = ret.substr( 0, ret.length()-1 );
        return ret;
    }


    static XalanDOMString sstr( const std::string& s )
    {
        return XalanDOMString( s.c_str() );
    }
    
};
