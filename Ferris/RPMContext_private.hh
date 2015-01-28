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

    $Id: RPMContext_private.hh,v 1.3 2010/09/24 21:30:56 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_RPM_CONTEXT_H_
#define _ALREADY_INCLUDED_FERRIS_RPM_CONTEXT_H_

#include <config.h>
#include <Ferris/HiddenSymbolSupport.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/RPM_private.hh>

namespace Ferris
{
    
    /**
     * root context for rpm://
     */
    class FERRISEXP_DLLLOCAL RPMRootContext
        :
        public Statefull_Recommending_ParentPointingTree_Context< RPMRootContext >
    {
        typedef RPMRootContext _Self;
        typedef Statefull_Recommending_ParentPointingTree_Context< RPMRootContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

    protected:

        virtual void priv_read();
        std::string getGroupName();
        virtual std::string priv_getMimeType( bool fromContent = false )
            { return MIMETYPE_DIRECTORY; }
        
    public:

        RPMRootContext();
        virtual ~RPMRootContext();

        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        void createStateLessAttributes( bool force = false );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    /**
     * Context for showing the base dir for an installed rpm package.
     */
    class FERRISEXP_DLLLOCAL RPMPackageContext
        :
        public StateLessEAHolder< RPMPackageContext, FakeInternalContext >
    {
        typedef RPMPackageContext _Self;
        typedef StateLessEAHolder< RPMPackageContext, FakeInternalContext > _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );
        bool m_readingDir;
        
    protected:

        virtual void priv_read();
        virtual std::string priv_getMimeType( bool fromContent = false )
            { return MIMETYPE_DIRECTORY; }

    public:

        RPMPackageContext( const fh_context& parent, const std::string& rdn );
        virtual ~RPMPackageContext();

        std::string getPackageName();

        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        void createStateLessAttributes( bool force = false );


        static fh_stringstream SL_getRPMHeaderString( RPMPackageContext*, int_32 );
        static fh_stringstream SL_getRPMPackage( RPMPackageContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMVersion( RPMPackageContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMRelease( RPMPackageContext*,const std::string&,EA_Atom*);
        static fh_stringstream SL_getRPMInfoURL( RPMPackageContext*,const std::string&,EA_Atom*);

        _Self* priv_CreateContext( Context* parent, std::string rdn );
        virtual void read( bool force = 0 );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    /**
     * Context for showing dirs/rpm packages.
     *
     * We have to overload the getURL()/getDirPath() methods to ensure
     * that the rpm groups are shown as the parent of the context instead
     * of the filesystem parents. ie. instead of /usr/local/doc/ferris-5/README
     * we need to give the path including the rpm group inforation in the 'parent'
     * context;
     * rpm://Development/C++/Ferris-5/usr/local/doc/ferris-5/README
     */
    class FERRISEXP_DLLLOCAL RPMContext
        :
        public ChainedViewContext
    {
        typedef RPMContext _Self;
        typedef ChainedViewContext _Base;
        
        virtual void priv_FillCreateSubContextSchemaParts( CreateSubContextSchemaPart_t& m );

    protected:

    public:

        RPMContext( const fh_context& parent, const fh_context& delegate );
        virtual ~RPMContext();

        virtual void read( bool force = 0 );
        
        virtual fh_context
        createSubContext( const std::string& rdn, fh_context md = 0 )
            throw( FerrisCreateSubContextFailed, FerrisCreateSubContextNotSupported );

        void createStateLessAttributes( bool force = false );

        virtual std::string getDirPath() throw (FerrisParentNotSetError);
        virtual std::string getURL();
        
        
        
        _Self*
        priv_CreateContext( Context* parent, std::string rdn )
            {
                return 0;
            }
    };

};
#endif
