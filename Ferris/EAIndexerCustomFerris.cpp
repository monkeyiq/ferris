/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2004 Ben Martin

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

    $Id: EAIndexerCustomFerris.cpp,v 1.4 2010/09/24 21:30:30 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include "config.h"
#include "EAIndexerMetaInterface.hh"

using namespace std;

namespace Ferris
{
    namespace EAIndex 
    {
        /************************************************************/
        /************************************************************/
        /************************************************************/

        namespace
        {
            MetaEAIndexerInterface* CreateEAIndexer()
            {
                return CreateEAIndexerFromLibrary( "libeaidxcustomferris.so" );
            }

            static const std::string MetaIndexClassName = "db4";
            static bool reged = MetaEAIndexerInterfaceFactory::Instance().
                Register( MetaIndexClassName, &CreateEAIndexer );
            static bool regedx = appendToMetaEAIndexClassNames( MetaIndexClassName );
        }

        /************************************************************/
        /************************************************************/
        /************************************************************/

        namespace
        {
            MetaEAIndexerInterface* CreateEAIndexer_db4tree()
            {
                return CreateEAIndexerFromLibrary( "libeaidxcustomferrisdb4tree.so" );
            }

            static const std::string MetaIndexClassName_db4tree = "db4tree";
            static bool reged_db4tree = MetaEAIndexerInterfaceFactory::Instance().
                Register( MetaIndexClassName_db4tree, &CreateEAIndexer_db4tree );
            static bool regedx_db4tree = appendToMetaEAIndexClassNames( MetaIndexClassName_db4tree );
        }

        /************************************************************/
        /************************************************************/
        /************************************************************/
        
    };
};


