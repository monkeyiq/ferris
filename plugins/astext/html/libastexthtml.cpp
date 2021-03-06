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

    $Id: libastexthtml.cpp,v 1.3 2010/09/24 21:31:31 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisAsTextPlugin.hh>
#include <Runner.hh>

using namespace std;

namespace Ferris
{

    class FERRISEXP_ASTEXT_PLUGIN AsTextStatelessFunctorPDF
        :
        public AsTextStatelessFunctor
    {
    public:
        virtual fh_istream getAsText( Context* c, const std::string& rdn, EA_Atom* atom )
            {
                fh_stringstream ss;

                string earl = c->getURL();
                if( starts_with( earl, "x-ferris" ))
                    earl = c->getDirPath();
                
                fh_runner r = new Runner();
                fh_stringstream cmdss;
                cmdss << "lynx -dump " //<< c->getDirPath()
                      << " \"" << earl << "\" "
                      << " ";
                LG_ATTR_D << "command:" << tostr(cmdss) << endl;
                r->setCommandLine( tostr(cmdss) );            
                r->setSpawnFlags(
                    GSpawnFlags(
                        r->getSpawnFlags() | 
                        G_SPAWN_SEARCH_PATH |
                        G_SPAWN_STDERR_TO_DEV_NULL |
                   0 ));
//                r->setConnectStdIn( true );
                r->Run();

                fh_istream stdoutss  = r->getStdOut();
                
                copy( istreambuf_iterator<char>(stdoutss),
                      istreambuf_iterator<char>(),
                      ostreambuf_iterator<char>(ss));

                int e = r->getExitStatus();
                return ss;

//                 fh_runner r = new Runner();
//                 fh_stringstream cmdss;
//                 cmdss << "lynx -dump " //<< c->getDirPath()
//                       << " - "
//                       << " ";
//                 cerr << "command:" << tostr(cmdss) << endl;
//                 r->setCommandLine( tostr(cmdss) );            
//                 r->setSpawnFlags(
//                     GSpawnFlags(
//                         G_SPAWN_SEARCH_PATH |
//                         G_SPAWN_STDERR_TO_DEV_NULL |
//                    0 ));
//                 r->setConnectStdIn( true );
//                 r->Run();

//                 fh_ostream toChildss = r->getStdIn();
//                 fh_istream stdoutss  = r->getStdOut();
//                 fh_istream htmlss = c->getIStream();
                
//                 copy( istreambuf_iterator<char>(htmlss),
//                       istreambuf_iterator<char>(),
//                       ostreambuf_iterator<char>(toChildss));
//                 toChildss << flush;
                
//                 copy( istreambuf_iterator<char>(stdoutss),
//                       istreambuf_iterator<char>(),
//                       ostreambuf_iterator<char>(ss));

//                 return ss;

            }
    };
    

    extern "C"
    {
        FERRISEXP_API fh_AsTextStatelessFunctor
        Create()
        {
            return new AsTextStatelessFunctorPDF();
        }
    };
};
