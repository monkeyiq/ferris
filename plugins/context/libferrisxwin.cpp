/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2005 Ben Martin

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

    $Id: libferrisxwin.cpp,v 1.5 2010/09/24 21:31:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>

#include <algorithm>
#include <numeric>

#include <config.h>

using namespace std;


#ifdef FERRIS_HAVE_ECORE
#include "libferrisxwin_ecore.hh"
#endif
#include "libferrisxwin_klipper.hh"

namespace Ferris
{
    extern "C"
    {
        FERRISEXP_EXPORT fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed );
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    

    /*
     * base bibtex file context
     */
    class FERRISEXP_CTXPLUGIN xwinRootContext
        :
        public FakeInternalContext
    {
        typedef xwinRootContext     _Self;
        typedef FakeInternalContext _Base;

    protected:

        virtual void priv_read()
            {
                staticDirContentsRAII _raii1( this );

                if( empty() )
                {
                    fh_fcontext loc = new FakeInternalContext( this, "localhost" );
                    Insert( GetImpl(loc), false, false );
                    
#ifdef FERRIS_HAVE_ECORE
                    {
                        fh_context c = new xwinTopDirectoryContext( GetImpl(loc), "window" );
                        loc->addNewChild( c );
                    }
#endif
                    {
                        fh_context c = new klipperTopDirectoryContext( GetImpl(loc), "clipboard" );
                        loc->addNewChild( c );
                    }
                }
            }
    public:
        
        xwinRootContext( Context* parent, const std::string& rdn )
            :
            _Base( parent, rdn )
            {
                createStateLessAttributes();
            }
        xwinRootContext()
            {
                createStateLessAttributes();
                createAttributes();
            }
        
        virtual ~xwinRootContext()
            {
            }
        virtual std::string priv_getRecommendedEA()
            {
                return "name";
            }
        virtual std::string getRecommendedEA()
            {
                return "name";
            }

        friend fh_context Brew( RootContextFactory* rf ) throw( RootContextCreationFailed );
        xwinRootContext* priv_CreateContext( Context* parent, string rdn )
            {
                xwinRootContext* ret = new xwinRootContext();
                ret->setContext( parent, rdn );
                return ret;
            }
        
    };
    

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

    extern "C"
    {
        fh_context Brew( RootContextFactory* rf )
            throw( RootContextCreationFailed )
        {
            try
            {
                static xwinRootContext c;
                fh_context ret = c.CreateContext( 0, rf->getInfo( "Root" ));
                return ret;
            }
            catch( exception& e )
            {
                LG_CTX_D << "Brew() e:" << e.what() << endl;
                Throw_RootContextCreationFailed( e.what(), 0 );
            }
        }
    }
    
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

};
