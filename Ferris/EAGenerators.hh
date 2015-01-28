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

    $Id: EAGenerators.hh,v 1.2 2010/09/24 21:30:29 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/


#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>

#ifndef _ALREADY_INCLUDED_EA_GENERATORS_H_
#define _ALREADY_INCLUDED_EA_GENERATORS_H_ 1

namespace Ferris
{
    

// class EAGenerator : public Versioned
// {
// protected:

//     Attribute*   Ctx;

//     virtual std::string priv_str() = 0;

// public:
    
//     EAGenerator( Attribute* ctx );
//     std::string str();
    

// };


// template <class T>
// class WrEAGenerator : public EAGenerator
// {
// protected:

//     Attribute*   Ctx;

//     virtual void priv_fromT( const T& s ) 
//         {
//         }

// public:
    
//     WrEAGenerator( Attribute* ctx )
//         : EAGenerator( ctx )
//         {}
    
//     void fromT( const T& s )
//         {
//             priv_fromT(s);
//         }
        
    

// };

// ///////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////


// class Digest_EAGenerator : public EAGenerator
// {
// protected:

//     virtual std::string priv_str();
//     std::string DigestName;
    
// public:

//     Digest_EAGenerator( Attribute* ctx, std::string _DigestName );
//     std::string getDigestName() 
//         {
//             return DigestName;
//         }
    
// };

// ///////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////////////////////////////////////////////////////


// class Head_EAGenerator : public WrEAGenerator<std::string>
// {
// protected: virtual std::string priv_str();
// public:    

//     Head_EAGenerator( Attribute* ctx  )
//         : WrEAGenerator<std::string>( ctx )
//         {}
    
    
// };

// class HeadRadix_EAGenerator : public WrEAGenerator<long>
// {
//     long V;

//     long getParam() 
//         {
//             return V;
//         }
    
// protected: virtual std::string priv_str();
// public:    

//     HeadRadix_EAGenerator( Attribute* ctx  )
//         : WrEAGenerator<long>( ctx ), V(16)
//         {}

//     typedef long value_t;
    
// };


///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

 
};


#endif // #ifndef _ALREADY_INCLUDED_EA_GENERATORS_H_
