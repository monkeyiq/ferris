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

    $Id: FerrisEAGeneratorPlugin_private.hh,v 1.3 2010/09/24 21:30:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EAGEN_PLUGIN_PRIV_H_
#define _ALREADY_INCLUDED_FERRIS_EAGEN_PLUGIN_PRIV_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <list>

namespace Ferris
{
    FERRISEXP_API void
    AppendAllStaticEAGeneratorFactories_Stateless(
        Context::s_StatelessEAGenFactorys_t& SL );
    
    FERRISEXP_API bool
    AppendAllStaticEAGeneratorFactories_Statefull(
        Context::m_StatefullEAGenFactorys_t& SF );

    FERRIS_SMARTPTR( StaticGModuleMatchedEAGeneratorFactory, fh_StaticMatchedEAGeneratorFactory );
    typedef std::list< fh_StaticMatchedEAGeneratorFactory > StaticEAGenFactorys_t;
    FERRISEXP_API StaticEAGenFactorys_t&
    getStaticLinkedEAGenFactorys();
    
//     FERRISEXP_API EAGenFactorys_t&
//     AppendAllStaticEAGeneratorFactories( EAGenFactorys_t& ret );
};

#endif

