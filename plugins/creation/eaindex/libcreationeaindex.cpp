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

    $Id: libcreationeaindex.cpp,v 1.3 2010/09/24 21:31:51 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>

#include <EAIndexer.hh>
#include <EAIndexer_private.hh>
using namespace std;

namespace Ferris
{
    using namespace EAIndex;
    
    class CreationStatelessFunctorEAIndex
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
            return new CreationStatelessFunctorEAIndex();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    
    fh_context
    CreationStatelessFunctorEAIndex::create( fh_context c, fh_context md )
    {
        try
        {
            EAIndex::fh_idx idx = createEAIndex( "db4", c, md );
        }
        catch( exception& e )
        {
            fh_stringstream ss;
            ss << "SL_SubCreate_eaindex() error creating index"
               << " URL:" << c->getURL()
               << " error:" << e.what()
               << endl;
            Throw_FerrisCreateSubContextFailed( tostr(ss), GetImpl(c) );
        }
        return c;
    }
};
