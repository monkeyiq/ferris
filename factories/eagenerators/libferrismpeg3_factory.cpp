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

    $Id: libferrismpeg3_factory.cpp,v 1.3 2010/09/24 21:31:29 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>

using namespace std;
namespace Ferris
{
    

static Factory::EndingList EL()
{
    static Factory::EndingList r;
    static bool virgin = true;

    if( virgin )
    {
        virgin = false;
        
        r.push_back( make_pair( string("name"), string(".mp3") ));
        r.push_back( make_pair( string("name"), string(".mpg") ));
        r.push_back( make_pair( string("name"), string(".mpeg") ));
        r.push_back( make_pair( string("name"), string(".avi") ));
        r.push_back( make_pair( string("name"), string(".mp2") ));
        r.push_back( make_pair( string("name"), string(".ac3") ));
        r.push_back( make_pair( string("name"), string(".ifo") ));
        r.push_back( make_pair( string("name"), string(".dat") ));
    }
    
    return r;
}



extern "C"
{
        bool hasState()
        {
            return true;
        }
    

    MatchedEAGeneratorFactory* MakeFactory()
    {
        return new GModuleMatchedEAGeneratorFactory(
             Factory::ComposeEndsWithMatcher( EL() ),
             AUTOTOOLS_CONFIG_LIBDIR + "/ferris/plugins/eagenerators/libferrismpeg3.so" );
    }
    
}
};
