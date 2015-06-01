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

    $Id: libastextextern.cpp,v 1.2 2010/09/24 21:31:31 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include "config.h"
#include <FerrisAsTextPlugin.hh>
#include <Runner.hh>
#include <config.h>

using namespace std;

namespace Ferris
{

    class FERRISEXP_ASTEXT_PLUGIN AsTextStatelessFunctorExtern
        :
        public AsTextStatelessFunctor
    {
    public:
        virtual fh_istream getAsText( Context* c, const std::string& rdn, EA_Atom* atom )
            {
//                string earl = c->getURL();
                string earl = c->getDirPath();
                LG_ASTEXT_D << "getAsText() url:" << earl << endl;
                
                fh_runner r = new Runner();
                fh_stringstream cmdss;
                string mimetype = getStrAttr( c, "mimetype", "" );
                if( false )
                {
                }
#ifdef HAVE_UNRTF
                else if( mimetype == "text/rtf" )
                {
                    cmdss << "libferris-astext-extern-extractor-rtf.sh ";
                }
#endif
#ifdef HAVE_HEVEA
                else if( mimetype == "text/tex" || mimetype == "text/x-tex" )
                {
                    cmdss << "libferris-astext-extern-extractor-tex.sh ";
                }
#endif
#ifdef HAVE_PS2ASCII
                else if( mimetype == "application/postscript" )
                {
                    cmdss << "libferris-astext-extern-extractor-ps.sh ";
                }
#endif
                else if( mimetype == "application/vnd.oasis.opendocument.text"
                         || mimetype == "application/vnd.oasis.opendocument.spreadsheet" )
                {
                    cmdss << "libferris-astext-extern-extractor-oo.sh ";
                }
                else if( mimetype == "application/x-kword"
                         || mimetype == "application/x-kspread" )
                {
                    cmdss << "libferris-astext-extern-extractor-koffice.sh ";
                }
                else
                {
                    stringstream ss;
                    ss << "Failed to find a shell script to convert file:" << earl
                       << " to plain text. mimetype:" << mimetype << endl;
                    Throw_CanNotGetStream( tostr(ss), c );
                }
                
                cmdss << " \"" << earl << "\" "
                      << " ";
                LG_ASTEXT_D << "command:" << tostr(cmdss) << endl;
                r->setCommandLine( tostr(cmdss) );            
                r->setSpawnFlags(
                    GSpawnFlags(
                        r->getSpawnFlags() | 
                        G_SPAWN_SEARCH_PATH |
                        G_SPAWN_STDERR_TO_DEV_NULL |
                   0 ));
                r->Run();

                fh_istream ret = r->getStdOut( true );
                LG_ASTEXT_D << "getAsText(have pipe stream for) url:" << earl << endl;
                return ret;
            }
    };
    

    extern "C"
    {
        FERRISEXP_API fh_AsTextStatelessFunctor
        Create()
        {
            return new AsTextStatelessFunctorExtern();
        }
    };
};
