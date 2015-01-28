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

    $Id: FerrisEAGeneratorPlugin.cpp,v 1.6 2010/09/24 21:30:36 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAGeneratorPlugin.hh>
#include <FerrisEAGeneratorPlugin_private.hh>

using std::cerr;
using std::endl;

namespace Ferris
{
    
    StaticEAGenFactorys_t& getStaticLinkedEAGenFactorys()
    {
        static StaticEAGenFactorys_t ret;
        return ret;
    }
    
    bool RegisterEAGeneratorModule(
        const fh_matcher& ma,
        const std::string& implname,
        const std::string& shortname,
        bool isDynamic,
        bool hasState,
        AttributeCreator::CreatePri_t CreatePri )
    {
        StaticEAGenFactorys_t& fac = getStaticLinkedEAGenFactorys();

//         std::cerr << "RegisterEAGeneratorModule() shortname:" << shortname
//                   << " isDyn:" << isDynamic
//                   << " hasState:" << hasState
//                   << " pri:" << CreatePri
//                   << std::endl;
        
        fac.push_back(
            new StaticGModuleMatchedEAGeneratorFactory( ma,
                                                        implname,
                                                        shortname,
                                                        isDynamic,
                                                        hasState,
                                                        CreatePri ));

        // DEBUG
//         cerr << "RegisterEAGeneratorModule() shortname:" << shortname << endl;
//         fh_context c = 0;
//         if( ma(c) )
//         {
//             std::cerr << "matches" << std::endl;
//         }
    }

    FERRISEXP_API void
    AppendAllStaticEAGeneratorFactories_Stateless(
        Context::s_StatelessEAGenFactorys_t& SL )
    {
        StaticEAGenFactorys_t& fac = getStaticLinkedEAGenFactorys();
        StaticEAGenFactorys_t::iterator end = fac.end();
        
        for( StaticEAGenFactorys_t::iterator fi = fac.begin(); fi != end; ++fi )
        {
            fh_StaticMatchedEAGeneratorFactory f = *fi;

            if( !f->hasState() )
                SL.push_back( f );
        }
    }
    
    FERRISEXP_API bool
    AppendAllStaticEAGeneratorFactories_Statefull(
        Context::m_StatefullEAGenFactorys_t& SF )
    {
        bool HaveDynamicAttributes = false;
        
        StaticEAGenFactorys_t& fac = getStaticLinkedEAGenFactorys();
        StaticEAGenFactorys_t::iterator end = fac.end();
        
        for( StaticEAGenFactorys_t::iterator fi = fac.begin(); fi != end; ++fi )
        {
            fh_StaticMatchedEAGeneratorFactory f = *fi;

            if( f->hasState() )
                SF.push_back( f->clone() );

            HaveDynamicAttributes |= f->isDynamic();
        }

        return HaveDynamicAttributes;
    }
    
    

    
//     EAGenFactorys_t& AppendAllStaticEAGeneratorFactories( EAGenFactorys_t& ret )
//     {
//         StaticEAGenFactorys_t& fac = getStaticLinkedEAGenFactorys();
//         StaticEAGenFactorys_t::iterator end = fac.end();
        
//         for( StaticEAGenFactorys_t::iterator fi = fac.begin(); fi != end; ++fi )
//         {
//             fh_StaticMatchedEAGeneratorFactory f = *fi;

//             if( f->hasState() )
//             {
//                 ret.push_back( f->clone() );
//             }
//             else
//             {
//                 ret.push_back( f );
//             }
//         }
//         return ret;
//     }
    
    
    
};
