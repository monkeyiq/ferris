/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2002 Ben Martin

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

    $Id: libcreationmarkup.cpp,v 1.2 2010/09/24 21:31:52 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>
#include <Runner.hh>

using namespace std;

namespace Ferris
{

    class CreationStatelessFunctorMarkup
        :
        public CreationStatelessFunctor
    {
    public:
        virtual fh_context create( fh_context c, fh_context md );
    };

    extern "C"
    {
        fh_CreationStatelessFunctor
        Create()
        {
            return new CreationStatelessFunctorMarkup();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    fh_context
    CreationStatelessFunctorMarkup::create( fh_context c, fh_context md )
    {
        string markupTypeName = md->getDirName();

        if( markupTypeName == "xml" )
        {
            fh_context newc = SubCreate_file( c, md );

            string rdn   = getStrSubCtx( md, "name", "" );
            string ename = getStrSubCtx( md, "root-element", "root" );
        
            fh_iostream ioss = newc->getIOStream();
            ioss << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>" << endl;
            ioss << "<" << ename << "/>" << endl;
            return newc;
        }

        fh_context newc = SubCreate_file( c, md );
        return newc;
    }
};
