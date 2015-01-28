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

    $Id: FerrisFileActions.hh,v 1.3 2010/09/24 21:30:37 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_FILE_ACTIONS_H_
#define _ALREADY_INCLUDED_FERRIS_FILE_ACTIONS_H_

#include <Ferris/TypeDecl.hh>

namespace Ferris
{
    namespace FileActions
    {
        class ExternalAppRunner;
        FERRIS_SMARTPTR( ExternalAppRunner, fh_ExternalAppRunner );

        enum KnownFileOperations {
            OP_VIEW = 1<<1,
            OP_EDIT = 1<<2,
            OP_DEFAULT = OP_VIEW,
            OP_UNKNOWN = 1<<15
        };

        FERRISEXP_API KnownFileOperations fromString( const std::string& s, KnownFileOperations def );
        
        /**
         * Perform a symbolic operation on the given collection of files
         * such as viewing them
         */
        FERRISEXP_API long ExecuteOperation( KnownFileOperations opcode, const ctxlist_t& l );

        /**
         * View/Edit are just two operations associated with a filetype
         * This gets the base context showing all the operations available
         * for the given selection
         */
        FERRISEXP_API fh_context getOpenWithContext( const ctxlist_t& l );
        
        /**
         * View/Edit are just two operations associated with a filetype
         */
        FERRISEXP_API long OpenWith( const std::string& n, const ctxlist_t& l );
        
        /**
         * Perform the symbolic application on the collection of files.
         * The application name has to exist in apps:// for this method to work
         */
        FERRISEXP_API long ExecuteApplicaionByShortcutName( std::string appname, ctxlist_t l,
                                                            KnownFileOperations opcode = OP_UNKNOWN );
        
        /**
         * Set the application to be executed for the given operation on the type
         * of file passed in arg1
         */
        FERRISEXP_API void setApplicationForOperationOnType( const ctxlist_t& l,
                                                             const std::string& appname,
                                                             KnownFileOperations opcode = OP_VIEW );
        FERRISEXP_API void setApplicationForOperationOnType( fh_context samplec,
                                                             const std::string& appname,
                                                             KnownFileOperations opcode = OP_VIEW );
        

        /**
         * Run an arbitrary executable command on the given collection of files.
         * This is a class to allow intricate adjustments to things like environment
         * variable inheritance and URL support possible.
         */
        class FERRISEXP_API ExternalAppRunner
            :
            public Handlable
        {
            std::string m_exename;
            bool m_supportsURL;
            bool m_opensMany;
            fh_context m_rootContext;
            stringlist_t m_environmentVariablesToInherit;
            KnownFileOperations m_opcode;
            bool m_openDirWhenNothingSelected;
            
            void setupDefaultInheritedEnvironmentNames();
            
        public:

            ExternalAppRunner( std::string exename, 
                               bool supportsURL = true ,
                               bool opensMany = true );
            
            void setSupportsURL( bool v );
            void setOpensMany( bool v );
            void setRootContext( fh_context c );
            
            stringlist_t& getEnvironmentVariablesToInherit();

            long run( const ctxlist_t& l );

            void setOpCode( KnownFileOperations opcode = OP_UNKNOWN );
            void setOpenDirWhenNothingSelected( bool v );
        };
    };
};
#endif

