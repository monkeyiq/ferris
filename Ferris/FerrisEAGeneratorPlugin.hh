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

    $Id: FerrisEAGeneratorPlugin.hh,v 1.3 2010/09/24 21:30:36 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_EAGEN_PLUGIN_H_
#define _ALREADY_INCLUDED_FERRIS_EAGEN_PLUGIN_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/MatchedEAGenerators.hh>

namespace Ferris
{
    /**
     * EA generator plugins are now (1.1.10)+ able to have their
     * factory module bootstrapped statically to libferris.so
     *
     * @ma the matcher to tell if this plugin is interested in making attributes
     * @implname Name of the main shared library for plugin
     * @shortname User presentable one word description
     * @isDynamic
     * @hasState
     * @CreatePri
     *
     */ 
    FERRISEXP_API bool RegisterEAGeneratorModule(
        const fh_matcher& ma,
        const std::string& implname,
        const std::string& shortname,
        bool isDynamic = false,
        bool hasState = false,
        AttributeCreator::CreatePri_t CreatePri =
        AttributeCreator::CREATE_PRI_NOT_SUPPORTED
        );
    
    
};

#endif
