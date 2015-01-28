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

    $Id: libferrisxwin_ecore.hh,v 1.2 2010/09/24 21:31:50 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#ifndef _ALREADY_INCLUDED_FERRIS_XWIN_ECORE_H_
#define _ALREADY_INCLUDED_FERRIS_XWIN_ECORE_H_

#include <FerrisContextPlugin.hh>
#include <Ferris/Ferris.hh>
#include <Ferris/TypeDecl.hh>
#include <Ferris/Trimming.hh>
#include <Ferris/General.hh>
#include <Ferris/Cache.hh>

#include <config.h>

extern "C" {
    #include <Ecore.h>
    #include <Ecore_X.h>
};

using namespace std;


namespace Ferris
{
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    typedef list< Ecore_X_Window > winlist_t;
    
    /*
     * 
     */
    class FERRISEXP_CTXPLUGIN xwinTopDirectoryContext
        :
        public FakeInternalContext
    {
        typedef xwinTopDirectoryContext  _Self;
        typedef FakeInternalContext      _Base;

        void getWinList( winlist_t& ret, Ecore_X_Window* el, int el_sz );
        void getWinList( winlist_t& ret );
        
    protected:

        virtual void priv_read();
    public:
        
        xwinTopDirectoryContext( Context* parent, const std::string& rdn );
        xwinTopDirectoryContext();
        virtual ~xwinTopDirectoryContext();
        virtual std::string priv_getRecommendedEA();
        virtual std::string getRecommendedEA();
        xwinTopDirectoryContext* priv_CreateContext( Context* parent, string rdn );
    };
    
    /*****************************************************************************/
    /******************************************************************************/
    /******************************************************************************/
};
#endif
