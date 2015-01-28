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

    $Id: Chmod.hh,v 1.2 2010/09/24 21:30:26 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef __GTK_FERRIS_CHMOD_H__
#define __GTK_FERRIS_CHMOD_H__

#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/TypeDecl.hh>

#include <string>

namespace Ferris
{
    class Chmod;
    FERRIS_SMARTPTR( Chmod, fh_chmod );

    namespace Factory
    {
        /**
         * Make a compiled object that can transform the mode of a context as
         * described by the given string
         */
        FERRISEXP_API fh_chmod MakeChmod( const std::string& s, long masked_ops = 0 );

        /**
         * Make a mode_t that can be used to set the initial mode of a context to
         * the mode closest to that described by the given string
         */
        FERRISEXP_API mode_t MakeInitializationMode( const std::string& s, long masked_ops = 0 );
    };

    struct FERRISEXP_DLLLOCAL ChmodOperation
    {
        mode_t mode;
        bool XifAnyX;
        bool CopyExisting;
        long AffectedBits;
        
        void setMode( mode_t v );
        void setModeXifAnyX( bool v );
        void setCopyExisting( bool v );
        void setAffectedBits( long v );
        ChmodOperation();

        /**
         * Used by children classes to get the mode_t to affect the
         * current object.
         */
        mode_t calcMode( mode_t existingmode, mode_t chained );
        
        
        
        /**
         * These operations can be chained together to perform the work
         * described in a string given to Factory::MakeChmod(). If a reference
         * mode string is given then that is passed as 'existing' and the return
         * value from one call is fed in as 'chained' to the next.
         *
         * The first call in the chain existing==chained.
         */
        virtual mode_t apply( mode_t existing, mode_t chained ) = 0;
    };
    
    
    class FERRISEXP_API Chmod
        :
        public Handlable
    {
        friend fh_chmod Factory::MakeChmod( const std::string& s, long );

        typedef std::list< ChmodOperation* > operations_t;
        operations_t operations;

        void append( ChmodOperation* o );
        
    public:

        virtual ~Chmod();
        mode_t operator()( mode_t m );
        inline mode_t apply( mode_t m ) { return this->operator()( m ); }

        /**
         * Get the mode_t that will set a new file/dir to the mode closest to
         * that given by the mode string
         */
        mode_t getInitializationMode();
    };
    

};

#endif
