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

    $Id: libastextwv.cpp,v 1.3 2010/09/24 21:31:31 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisAsTextPlugin.hh>
#include <Runner.hh>

using namespace std;

namespace Ferris
{

    class FERRISEXP_ASTEXT_PLUGIN AsTextStatelessFunctorWV
        :
        public AsTextStatelessFunctor
    {
    public:
        virtual fh_istream getAsText( Context* c, const std::string& rdn, EA_Atom* atom )
            {
//                cerr << "getAsText() url:" << c->getURL() << endl;
                
                string earl = c->getURL();
//                if( starts_with( earl, "x-ferris" ))
                earl = c->getDirPath();

                string tmpfilename = tmpnam(0);

                fh_runner r = new Runner();
                fh_stringstream cmdss;
                cmdss << "wvText " //<< c->getDirPath()
                      << " \"" << earl << "\" "
                      << " \"" << tmpfilename << "\" "
                      << " ";
                LG_ATTR_D << "command:" << tostr(cmdss) << endl;
                r->setCommandLine( tostr(cmdss) );            
                r->setSpawnFlags(
                    GSpawnFlags(
                        r->getSpawnFlags() | 
                        G_SPAWN_SEARCH_PATH |
                        G_SPAWN_STDERR_TO_DEV_NULL |
                        G_SPAWN_STDOUT_TO_DEV_NULL |
                   0 ));
                r->Run();
                int e = r->getExitStatus();

//                cerr << "tmpfilename:" << tmpfilename << endl;
                
                fh_fstream ret( tmpfilename );
                unlink( tmpfilename.c_str() );
                return ret;
            }
    };
    

    extern "C"
    {
        FERRISEXP_API fh_AsTextStatelessFunctor
        Create()
        {
            return new AsTextStatelessFunctorWV();
        }
    };
};
