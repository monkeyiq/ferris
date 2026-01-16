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

    $Id: FerrisContextPlugin.hh,v 1.2 2010/09/24 21:31:32 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CONTEXT_PLUGIN_H_
#define _ALREADY_INCLUDED_FERRIS_CONTEXT_PLUGIN_H_

#include <config.h>

#include <Ferris/HiddenSymbolSupport.hh>

#include <Ferris.hh>
#include <SignalStreams.hh>
#include <Ferris_private.hh>
#include <SM.hh>
#include <Context.hh>

#include <fstream>

#include <sigc++/sigc++.h>

#include <Resolver_private.hh>


#define FERRISEXP_CTXPLUGIN FERRISEXP_DLLLOCAL

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////


extern "C"
{
    FERRISEXP_DLLLOCAL Ferris::fh_context
    Brew( Ferris::RootContextFactory* rf );
}



#endif // #ifndef _ALREADY_INCLUDED_FERRIS_CONTEXT_PLUGIN_H_
