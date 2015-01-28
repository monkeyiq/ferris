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

    $Id: FerrisCreationPlugin.hh,v 1.2 2010/09/24 21:30:35 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_CREATION_PLUGIN_H_
#define _ALREADY_INCLUDED_FERRIS_CREATION_PLUGIN_H_

#include <Ferris/HiddenSymbolSupport.hh>

//
// Its a PRIVATE header, so config.h is OK
//
#include <config.h>

#include <Ferris.hh>
#include <SignalStreams.hh>
#include <Ferris_private.hh>
#include <SM.hh>
#include <Context.hh>

#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#include <sigc++/sigc++.h>


/**
 * The factory calls the register functions to tell the main code where to
 * find the function to create a new context of the given type.
 *
 * once the library is known it is opened and a function is called in that
 * library to make the new context.
 *
 * The registry stores the ferris-type, XSD for the new type, and the library name
 * once a match is found the library is opened and a new object is created.
 */
namespace Ferris
{
    FERRISEXP_API mode_t getModeFromMetaData( fh_context md );

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    /**
     * There should be a function in the above registered library 'libpath'
     * that can create one of these objects. The object is then cached and
     * used by future creation requests of the ferris-type
     */
    class FERRISEXP_API CreationStatelessFunctor
        :
        public Handlable
    {
    protected:
        fh_context SubCreate_file( fh_context c, fh_context md );
        fh_context SubCreate_ea  ( fh_context c, fh_context md );

    public:
        virtual fh_context create( fh_context c, fh_context md ) = 0;
    };
    FERRIS_SMARTPTR( CreationStatelessFunctor, fh_CreationStatelessFunctor );

    /**
     * All creation factories should call here to register their shared object.
     * Note that many calls to RegisterCreationModule() can be made for the one
     * shared object. In such cases the creation method can demultiplex based on
     * the rdn of the metadata supplied. See the ImageMagick creator as an example.
     *
     * @param libname Name of shared object that can create new Context(s) of
     *                type ferristype
     * @param ferristype Ferris ID of new Context that can be created by shared
     *                   object.
     * @param xsd XSD schema that the creation can accept.
     * @param requiresNativeKernelDrive If the creation process uses shared libraries
     *                                  that require direct access to the filesystem
     *                                  to perform creation then set this to true
     *                                  (the default). Otherwise set it to false so that
     *                                  the creator is registered for any underlying VFS
     *                                  context module. eg. if set to false this creation
     *                                  method will be allowed for embedded db4 or mysql
     *                                  filesystem.
     * @param simpleTypes If there are any XSD types that are used in 'xsd' to restrict
     *                    the valid values that a document instance can have then much
     *                    XSD 'simpleType' tags should be contained in this string.
     */
    FERRISEXP_API bool RegisterCreationModule( const std::string& libname,
                                 const std::string& ferristype,
                                 const std::string& xsd,
                                 bool requiresNativeKernelDrive = true,
                                 const std::string& simpleTypes = "" );

    /**
     * If a plugin registers multiple types for the one library, it should
     * call here with any XSD SimpleTypes that it wants rather than
     * registering the same simpletypes many times.
     */
    FERRISEXP_API bool appendExtraGenerateSchemaSimpleTypes( const std::string& s );
    
    /**
     * Insert all creation modules no matter if they need native disk or not.
     */
    FERRISEXP_API void insertAllCreatorModules( Context::CreateSubContextSchemaPart_t& m );

    /**
     * Insert only to creation modules that don't require native kernel disk IO.
     */
    FERRISEXP_API void insertAbstractCreatorModules( Context::CreateSubContextSchemaPart_t& m );
};

#endif
