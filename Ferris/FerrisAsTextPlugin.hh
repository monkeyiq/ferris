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

    $Id: FerrisAsTextPlugin.hh,v 1.3 2010/09/24 21:30:34 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_ASTEXT_PLUGIN_H_
#define _ALREADY_INCLUDED_FERRIS_ASTEXT_PLUGIN_H_

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/SignalStreams.hh>
#include <Ferris/Ferris_private.hh>
#include <Ferris/SM.hh>
#include <Ferris/Context.hh>

#include <fstream>

#include <sigc++/sigc++.h>

#define FERRISEXP_ASTEXT_PLUGIN FERRISEXP_DLLLOCAL

/**
 * The factory calls the register functions to tell the main code where to
 * find the function to handle the mime type (ie, a mapping from mime/ferris
 * type to shared library is created by the _factory.cpp files).
 *
 * one the library is known it is opened and a function is called in that
 * library to make the text version.
 */
namespace Ferris
{
    FERRISEXP_API bool RegisterAsTextFromMime( const std::string& mimetype,
                                               const std::string& libname );
    FERRISEXP_API bool RegisterAsTextFromFerrisType( const std::string& ftype,
                                                     const std::string& libname );
    FERRISEXP_API bool RegisterAsTextFromMatcher(
        const fh_matcher& ma,
        const std::string& libname );

    FERRISEXP_API std::string getLibraryNameFromMime      ( const std::string& mimetype );
    FERRISEXP_API std::string getLibraryNameFromFerrisType( const std::string& ftype    );
    FERRISEXP_API std::string getLibraryNameFromMatcher   ( fh_context& c );
    
    /**
     * There should be a function in the above registered library 'libpath'
     * that can create one of these objects. The object is then cached and
     * used by future "as-text" requests in ferris.cpp
     */
    class FERRISEXP_API AsTextStatelessFunctor
        :
        public Handlable
    {
    public:
        virtual fh_istream getAsText( Context* c, const std::string& rdn, EA_Atom* atom ) = 0;
    };
    FERRIS_SMARTPTR( AsTextStatelessFunctor, fh_AsTextStatelessFunctor );

    
//     typedef Loki::SingletonHolder<
//         Loki::Factory< std::string, std::string > > AsTextFactory_t;

//     typedef AsTextFactory_t AsTextFromMimeFactory;
//     typedef AsTextFactory_t AsTextFromFerrisTypeFactory;
    
//     template < class Base,class Sub >
//     struct MakeObject
//     {
//         static Base* Create()
//             { return new Sub(); }
//     };
};

#endif
