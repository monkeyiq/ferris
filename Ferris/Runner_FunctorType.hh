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

    $Id: Runner_FunctorType.hh,v 1.3 2010/09/24 21:30:58 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_RUNNER_FUN_H_
#define _ALREADY_INCLUDED_FERRIS_RUNNER_FUN_H_

#include <Ferris/SignalStreams.hh>

namespace Ferris
{
    typedef Loki::Functor< fh_istream,
                           LOKI_TYPELIST_2( fh_runner, fh_istream ) > Runner_AsyncIOFunctor_t;
};

#endif
