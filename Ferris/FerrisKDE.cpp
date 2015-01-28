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

    $Id: FerrisKDE.cpp,v 1.7 2011/07/31 21:30:49 ben Exp $

    *******************************************************************************
    *******************************************************************************
    ******************************************************************************/

#include "config.h"
#include "FerrisKDE.hh"

#include <Ferris.hh>
#include <FerrisQt_private.hh>
#include <Native.hh>

#ifdef HAVE_KDE
#include <qapplication.h>
#include <kaboutdata.h>
#include <kmimetype.h>
#include <kdebug.h>
#include <kapplication.h>
#include <kcmdlineargs.h>
#endif



using namespace std;


namespace Ferris
{
    namespace KDE
    {
#ifdef HAVE_KDE

        /**
         * This exists because folks might want to use libferris compiled
         * with KDE mime sniffing from an app which is not a KDE client.
         *
         * ie. We dont want to force command line clients to know anything
         * about KDE if they just want to getStrAttr( ctx, "mimetype" );
         */
        class FERRISEXP_DLLLOCAL KAppHolder
        {
            KApplication& getKApp()
                {
                    static KApplication* a = new KApplication( true );
                    return *a;
                }

        public:
            KAppHolder()
                {
                    if( !KApplication::kApplication() )
                    {
                        KLocalizedString kl = KLocalizedString();
                        QByteArray qba = "libferris";
                        KAboutData* kabout = new KAboutData( qba, qba, kl, qba );
                        int argc = 1;
                        char* argv[] = { "libferris", 0 };
                        KCmdLineArgs::init( argc, argv, kabout );
                        
                        getKApp();
                    }
                }
        };

        typedef Loki::SingletonHolder< KAppHolder, Loki::CreateUsingNew, Loki::NoDestroy  > KAppSingleton;
#endif


        void ensureKDEApplication()
        {
            installQTMsgHandler();
#ifdef HAVE_KDE
            if(getenv("DISPLAY") && strlen(getenv("DISPLAY")))
                KAppSingleton::Instance();
            else
                ensureQApplication();
#else
            ensureQApplication();
#endif
        }
        
        std::string guessMimeType( const std::string& path )
        {
#ifdef HAVE_KDE
            KAppSingleton::Instance();
            KUrl u;
            u.setPath( path.c_str() );
            KMimeType::Ptr type = KMimeType::findByUrl( u );
            return type->name().toUtf8().data();
#endif
            return "document/unknown";
        }
        
        string getMimeType( fh_context c, bool fromContent  )
        {
#ifdef HAVE_KDE
            KAppSingleton::Instance();

            if( fromContent )
            {
                const int sz = 4096;
                QByteArray array( sz, 1 );

                fh_istream iss = c->getIStream();
                iss.read( array.data(), sz );
            
                KMimeType::Ptr type = KMimeType::findByContent( array );
                return type->name().toUtf8().data();
            }
            else
            {
                KUrl u;
                u.setPath( c->getDirPath().c_str() );
                KMimeType::Ptr type = KMimeType::findByUrl( u );
                return type->name().toUtf8().data();
            }
#endif
            return "document/unknown";
        }
    };
    
};
