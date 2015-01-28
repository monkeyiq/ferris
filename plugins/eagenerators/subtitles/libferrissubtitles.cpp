/******************************************************************************
*******************************************************************************
*******************************************************************************

    libferris
    Copyright (C) 2010 Ben Martin

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    For more details see the COPYING file in the root directory of this
    distribution.

    $Id: libferrismediainfo.cpp,v 1.1 2007/08/15 21:30:09 ben Exp $

*******************************************************************************
*******************************************************************************
******************************************************************************/

#include <FerrisEAPlugin.hh>
#include <FerrisBoost.hh>
#include <Ferris/General.hh>
#include <Ferris/Runner.hh>


using namespace std;

#define DEBUG LG_SUBTITLES_D

namespace Ferris
{
    
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

    static string CanonEAName( const std::string& s )
    {
        string ret = tolowerstr()( s );
        return ret;
    }

    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/
    /********************************************************************************/

    class FERRISEXP_DLLLOCAL EAGenerator_Subtitles : public MatchedEAGeneratorFactory
    {
        typedef EAGenerator_Subtitles _Self;
        
    protected:

        virtual void Brew( const fh_context& a );

    public:


        fh_istream get_subtitles( Context* c, const std::string& eaname, EA_Atom* atom )
        {
            fh_stringstream ss;

            fh_runner r = new Runner();
            r->pushCommandLineArg( "ferris-internal-extract-subtitles-to-format");
            r->pushCommandLineArg( "default");
            r->pushCommandLineArg( "txt");
            r->pushCommandLineArg( c->getURL() );

            std::string subtitles;
            int e = r->executeAndReturnStdOut( subtitles );
        
            DEBUG << "subtitles.sz:" << subtitles.length() << " url:" << c->getURL() << endl;
            DEBUG << "subtitles:" << subtitles << endl;
            DEBUG << "e:" << e << " url:" << c->getURL() << " complete" << endl;
            ss << subtitles;
            return ss;
        }
        
        
        EAGenerator_Subtitles()
            :
            MatchedEAGeneratorFactory()
        {
        }

        virtual bool tryBrew( const fh_context& ctx, const std::string& eaname );
    };


    
    void
    EAGenerator_Subtitles::Brew( const fh_context& a )
    {
        DEBUG << "EAGenerator_Subtitles::Brew() url:" << a->getURL() << endl;

        static bool brewing = false;
        if( brewing )
            return;
        Util::ValueRestorer< bool > dummy1( brewing, true );

        
#if 0
        // fh_runner r = new Runner();
        // r->pushCommandLineArg( "ferris-internal-extract-subtitles-to-format");
        // r->pushCommandLineArg( "default");
        // r->pushCommandLineArg( "txt");
        // r->pushCommandLineArg( a->getURL() );

        // std::string subtitles;
        // int e = r->executeAndReturnStdOut( subtitles );
        
        // DEBUG << "Brew() subtitles.sz:" << subtitles.length() << " url:" << a->getURL() << endl;
        // DEBUG << "Brew() subtitles:" << subtitles << endl;
        // a->addAttribute( "subtitles", subtitles, XSD_BASIC_STRING );
#endif

        a->addAttribute( "subtitles-local", this, &_Self::get_subtitles, XSD_BASIC_STRING );

        DEBUG << "Brew() url:" << a->getURL() << " complete" << endl;
    }

    bool
    EAGenerator_Subtitles::tryBrew( const fh_context& ctx, const std::string& eaname )
    {
        DEBUG << "tryBrew() " << " url:" << ctx->getURL() << endl;

        Brew( ctx );
        return ctx->isAttributeBound( eaname, false );
    }

    extern "C"
    {
        FERRISEXP_EXPORT MatchedEAGeneratorFactory* CreateRealFactory()
        {
            DEBUG << "EAGenerator_Subtitles::CreateRealFactory()" << endl;
            return new EAGenerator_Subtitles();
        }
    };
};
