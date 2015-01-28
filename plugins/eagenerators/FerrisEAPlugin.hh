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

    $Id: FerrisEAPlugin.hh,v 1.3 2010/09/24 21:31:54 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <Ferris/HiddenSymbolSupport.hh>

#include <Ferris.hh>
#include <FerrisEAGeneratorPlugin.hh>
#include <Attribute.hh>
#include <Attribute_private.hh>
#include <SignalStreams.hh>
#include <sigc++/sigc++.h>
#ifdef SIGC_CXX_NAMESPACES
using namespace SigC;
#endif
#define SIGCXX_SIGNAL_SYSTEM_H 1

#include <fstream>
#include <Ferris_private.hh>
#include <SM.hh>


#include "config.h"
