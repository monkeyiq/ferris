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

    $Id: libcreationdevsource.cpp,v 1.2 2010/09/24 21:31:51 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include <FerrisCreationPlugin.hh>
#include <Runner.hh>

using namespace std;

namespace Ferris
{

    class CreationStatelessFunctorDevSource
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
            return new CreationStatelessFunctorDevSource();
        }
    };

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    
    fh_context
    CreationStatelessFunctorDevSource::create( fh_context c, fh_context md )
    {
        fh_context newc        = SubCreate_file( c, md );
        string rdn             = newc->getDirName();
        string hrdn            = rdn.substr( 0, rdn.find('.'));
        string SourceTypeName  = md->getDirName();
        string LicenseTypeName = getStrSubCtx( md, "license", "GPL" );

        fh_iostream newc_ss = newc->getIOStream();
        fh_stringstream rss;
        rss << "~/.ferris/licenses/" << "src-" << SourceTypeName << "-" << LicenseTypeName;
        fh_context lc = Resolve( tostr(rss) );

        fh_runner r   = new Runner();
        r->setSpawnFlags( GSpawnFlags( r->getSpawnFlags()
                                       | G_SPAWN_STDERR_TO_DEV_NULL
                                       | G_SPAWN_SEARCH_PATH) );
        r->getArgv().push_back( lc->getDirPath() );
        
        {
            fh_stringstream ss;
            ss << "STARTOFNAME=" << hrdn;
            r->getEnvp().push_back( tostr(ss) );
        }
        {
            fh_stringstream ss;
            ss << "RDN=" << rdn;
            r->getEnvp().push_back( tostr(ss) );
        }
        {
            fh_stringstream ss;
            ss << "NEWPATH=" << newc->getDirPath();
            r->getEnvp().push_back( tostr(ss) );
        }
        

        
        r->Run();
        LG_FERRISCREATE_D << "execution of cat was OK" << endl;
        fh_istream stdoutss =  r->getStdOut();
        std::copy( std::istreambuf_iterator<char>(stdoutss),
                   std::istreambuf_iterator<char>(),
                   std::ostreambuf_iterator<char>(newc_ss));

        return newc;
    }
};
