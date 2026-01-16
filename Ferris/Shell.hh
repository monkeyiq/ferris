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

    $Id: Shell.hh,v 1.13 2010/09/24 21:30:59 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __GTK_FERRIS_SHELL_H__
#define __GTK_FERRIS_SHELL_H__

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Chmod.hh>
#include <Ferris/FerrisException.hh>

#include <pwd.h>

#include <string>

namespace Ferris
{
    /**
     * Test to see if the context 'p' is a transitive parent of the context 'base'.
     *
     * @param  p    The context that should be a transitive parent of base
     * @param  base The context that should be a transitive child of p
     * @return true of p is reachable as a parent from base or false otherwise
     */
    FERRISEXP_API bool isTransitiveParent( fh_context base, fh_context p );


    /**
     * Tell libferris that it will be running set UID.
     */
    FERRISEXP_API void setRunningSetUID( bool v );

    /**
     * Some functionality in libferris is dropped out when
     * running set UID.
     */
    FERRISEXP_API bool runningSetUID();

    /**
     * Try to work out as quickly as possible if the given URL can
     * be resolved. For file:// URLs this might use lstat() instead
     * of Resolve() to get that little bit of extra speed.
     */
    FERRISEXP_API bool canResolve( const std::string& s );

    /**
     * Performs the same action as readlink --canonicalize-missing on the URL
     */
    FERRISEXP_API std::string canonicalizeMissing( const std::string& earl );

    
    /**
     * Note that the use of ContextCreated_Sig_t is entirely optional. It is there
     * mainly to allow clients to have full access to an audit trail of mkdir -p
     * type actions.
     *
     * Usage of the 'int mode' param is also optional, sane defaults are provided
     * which allow fairly restricted access by default. Use MakeChmodMode() to parse
     * a standard mode spec to obtain the normalized mode for use in parameters.
     */
    namespace Shell
    {
        typedef sigc::signal< void ( fh_context ) > ContextCreated_Sig_t;
        FERRISEXP_API ContextCreated_Sig_t& getNullContextCreated_Sig();

        /**
         * Quote the given string for use in a shell if it needs quoting.
         */
        FERRISEXP_API std::string quote( const std::string& s );

        /**
         * If c is a soft/hardlink then return what it points to.
         * if what it points to is also a link then resolve that again.
         *
         * @param throwForException if true then throw errors to the caller,
         *        otherwise if there is a problem resolving one of
         *        the links then just return the last context in the resolution
         * @param levelOfRecursion max number of times to try to find a non
         *        link target.
         * @returns the last attempt to find a non link target.
         */
        FERRISEXP_API fh_context unrollLinks( fh_context c,
                                bool throwForException = true,
                                int levelOfRecursion = 255 );

        FERRISEXP_API std::string getCWDString();
        FERRISEXP_API fh_context  getCWD();
        FERRISEXP_API const fh_context& setCWD(fh_context& ctx);

        /**
         * Create a new name/value EA for the given context.
         */
        FERRISEXP_API void createEA( fh_context c,
                                     const std::string& name,
                                     const std::string& value );

        /**
         * Sometimes the plugin used should be explicitly specified. For example,
         * copying a file from an NFS share to a local disk, most user written EA
         * will be in RDF for the source file and we would like to preserve this
         * in the destination if possible by writing that same metadata into RDF
         * again.
         */
        FERRISEXP_API void createEA( fh_context c,
                                     const std::string& name,
                                     const std::string& value,
                                     const std::string& explicitPluginShortName );

        /**
         * For files which are overmounted, for example, the path /tmp/example.xml,
         * we want the ability to create an extended attribute on the file itself
         * rather than delegating to the XML handler.
         */
        FERRISEXP_API void createEA( fh_context c,
                                     const std::string& name,
                                     const std::string& value,
                                     bool dontDelegateToOvermountContext );

        /**
         * Ensure that a given EA exists for the context and has the given value.
         */
        FERRISEXP_API void ensureEA( fh_context c,
                                     const std::string& name,
                                     const std::string& value );
        FERRISEXP_API fh_context CreateFile( fh_context c,
                                             const std::string& n,
                                             int mode = 0,
                                             ContextCreated_Sig_t& sigh =
                                             Ferris::Shell::getNullContextCreated_Sig() );
        FERRISEXP_API fh_context CreateDB4( fh_context c,
                                            const std::string& n,
                                            int mode = 0,
                                            ContextCreated_Sig_t& sigh =
                                            Ferris::Shell::getNullContextCreated_Sig() );
        FERRISEXP_API fh_context EnsureDB4( const std::string& path, const std::string& n );
        FERRISEXP_API fh_context CreateLink( fh_context existingc,
                               fh_context newc_parent,
                               const std::string& newrdn,
                               bool useURL = false,
                               bool isSoft = true,
                               ContextCreated_Sig_t& sigh =
                               Ferris::Shell::getNullContextCreated_Sig() );
        FERRISEXP_API fh_context CreateDir( fh_context c,
                              const std::string& n,
                              bool WithParents = false,
                              int mode = 0,
                              ContextCreated_Sig_t& sigh =
                              Ferris::Shell::getNullContextCreated_Sig() );
        FERRISEXP_API fh_context CreateDir( const std::string& path,
                              bool WithParents = false,
                              int mode = 0,
                              ContextCreated_Sig_t& sigh =
                              Ferris::Shell::getNullContextCreated_Sig() );
        FERRISEXP_API fh_context acquireSubContext( fh_context parent,
                                      const std::string& rdn,
                                      bool isDir = false,
                                      int mode = 0,
                                      ContextCreated_Sig_t& sigh =
                                      Ferris::Shell::getNullContextCreated_Sig() );


        /**
         * get the directory context at path, create it if it doesn't exist yet.
         */
        FERRISEXP_API fh_context acquireContext( std::string path,
                                                 int mode = 0,
                                                 bool isDir = true,
                                                 ContextCreated_Sig_t& sigh =
                                                 Ferris::Shell::getNullContextCreated_Sig() );


        FERRISEXP_API std::string getHomeDirPath_nochecks();
        FERRISEXP_API std::string getHomeDirPath();
        FERRISEXP_API std::string getCWDDirPath();
        FERRISEXP_API void setCWDDirPath( const std::string& p );
        FERRISEXP_API std::string getTmpDirPath();

        FERRISEXP_API bool contextExists( const std::string& path );


        /**
         * lookup the numerical user ID for a user with a given login name
         * @param name Name of user to find numerical ID for. default is
         * user running current proccess.
         */
        FERRISEXP_API uid_t getUserID( const std::string& name = "" );

        /**
         * lookup the numerical group ID for a user with a given login name
         * @param name Name of user to find numerical ID for. default is
         * user running current proccess.
         */
        FERRISEXP_API gid_t getGroupID( const std::string& name = "" );

        /**
         * lookup a user name for a given user ID
         */
        FERRISEXP_API std::string getUserName( uid_t id );
        /**
         * lookup a group name for a given group ID
         */
        FERRISEXP_API std::string getGroupName( gid_t id );

        

        /**
         * touch a context updating the mtime, atime or both.
         * Optionally create the target context as either a file
         * by default or a directory if isDir is true. The initial
         * mode for the new object(s) can be set using the mode arg.
         * Control over what time values are updated and to what value
         * is exposed with the touch?Time, use?Time args.
         *
         * @return The context that was touched
         * @arg path The path of the context to touch
         * @arg create if true and the target context doesn't exist
         *             then it will be created first
         * @arg isDir  If new context(s) are required set this to true to
         *             create directories instead of files by default
         * @arg mode   chmod like protection for new objects
         * @arg touchMTime Should mtime be updated
         * @arg touchATime Should atime be updated
         * @arg useMTime set the modification time value to set the object
         *               at path to. A default value of zero means to set
         *               to time to Time::getTime(), ie the current time.
         * @arg useATime set the atime time value to set the object
         *               at path to. A default value of zero means to set
         *               to time to Time::getTime(), ie the current time.
         *
         *
         */
        FERRISEXP_API fh_context touch( const std::string& path,
                                        bool create     = true,
                                        bool isDir      = false,
                                        int  mode       = 0,
                                        bool touchMTime = true,
                                        bool touchATime = true,
                                        time_t useMTime = 0,
                                        time_t useATime = 0 );
        FERRISEXP_API fh_context touch( const std::string& path,
                                        const std::string& SELinux_context,
                                        bool create     = true,
                                        bool isDir      = false,
                                        int  mode       = 0,
                                        bool touchMTime = true,
                                        bool touchATime = true,
                                        time_t useMTime = 0,
                                        time_t useATime = 0 );

        /**
         * Generate a unique tempfile with the given path prefix.
         * the given string is updated and the opened file is returned.
         */
        fh_iostream generteTempFile( std::string& templateStr, bool closeFD = true );
        int         generateTempFD( const char* templateStrReadOnly );
        int         generateTempFD( std::string& templateStr );
        int         generateTempFD();
        fh_iostream generteTempFile( bool closeFD = true );
        fh_context  generateTempDir( std::string& templateStr );
        
    };

    
};
#endif
