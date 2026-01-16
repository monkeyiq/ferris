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

    $Id: ContextPlugin.hh,v 1.3 2010/09/24 21:30:27 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CONTEXT_PLUGIN_H_
#define _ALREADY_INCLUDED_FERRIS_CONTEXT_PLUGIN_H_

#include "Ferris/Ferris.hh"
#include "Ferris/Resolver_private.hh"

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf );
    };
};

#define DYNAMICLINKED_ROOTCTX_CLASSCHUNK( KLASS )                       \
    friend fh_context Brew( RootContextFactory* rf ); \
    KLASS* priv_CreateContext( Context* parent, string rdn )            \
    {                                                                   \
        KLASS* ret = new KLASS();                                       \
        ret->setContext( parent, rdn );                                 \
        return ret;                                                     \
    }


#define DYNAMICLINKED_ROOTCTX_DROPPER( URISCHEME, KLASS )               \
    extern "C"                                                          \
    {                                                                   \
        fh_context Brew( RootContextFactory* rf )                       \
        {                                                               \
            try                                                         \
            {                                                           \
                static fh_context ret = new KLASS();                    \
                return ret;                                             \
            }                                                           \
            catch( exception& e )                                       \
            {                                                           \
                LG_CTX_ER << "Brew() e:" << e.what() << endl;           \
                Throw_RootContextCreationFailed( e.what(), 0 );         \
            }                                                           \
        }                                                               \
    }


#endif

